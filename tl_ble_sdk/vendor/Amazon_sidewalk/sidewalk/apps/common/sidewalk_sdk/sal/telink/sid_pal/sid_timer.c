/********************************************************************************************************
 * @file    sid_timer.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    11,2025
 *
 * @par     Copyright (c) 2025, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/
/** @file sid_timer.c
 *  @brief Timer interface implementation.
 */
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "sid_ble_adapter.h"
#include <sid_pal_timer_ifc.h>
#include <sid_pal_uptime_ifc.h>
#include <sid_pal_assert_ifc.h>
#include <sid_pal_critical_region_ifc.h>
#include <sid_time_ops.h>
#include <stdint.h>
#include <lk/list.h>
#if (FREERTOS_ENABLE)
    #include "tlk_riscv.h"
    #include <FreeRTOS.h>
    #include <task.h>
    #include <timers.h>
    #include <queue.h>
    #include <semphr.h>
    #include <event_groups.h>
    #include "app_freertos.h"
#endif


#ifdef CONFIG_SIDEWALK_THREAD_TIMER
static StaticSemaphore_t xTimerTriggerSemBuf;
static SemaphoreHandle_t xTimerTriggerSem = NULL;
#endif

struct sid_pal_timer_ctx{
    list_node_t list;
};

static const struct sid_timespec tolerance_lowpower = { .tv_sec = 1, .tv_nsec = 0 };
static const struct sid_timespec tolerance_precise = { .tv_sec = 0, .tv_nsec = 0 };
static uint8_t sid_timer_sleep_flag = 0;

#if (CONFIG_TIMER_SOURCE == TIMER_SOURCE_OS)
static TimerHandle_t       g_xNextTimer;
static StaticTimer_t       g_xNextTimerBuffer;
#endif

static struct sid_pal_timer_ctx sid_pal_timer_ctx = { .list = LIST_INITIAL_VALUE(sid_pal_timer_ctx.list), };

static void sid_timer_start(struct sid_timespec *sid_time);
static void sid_timer_resche( struct sid_timespec *when);
void sid_pal_timer_event_callback_l(void * arg, const struct sid_timespec * now);

#define NS_TO_32K_RC_COUNTER 31250
#define NS_TO_32K_XTAL_COUNTER 30518
#define SID_TIMER_THRELD_MAX_32K_US  5000   //  100ms 32759/1000

#define SID_TIMER_MODE 0

uint8_t sid_timer_is_forbidden_sleep(void)
{
    return sid_timer_sleep_flag;
}


static const struct sid_timespec *sid_pal_timer_get_tolerance(sid_pal_timer_prio_class_t type)
{
    const struct sid_timespec *tolerance = NULL;

    switch (type) {
    case SID_PAL_TIMER_PRIO_CLASS_PRECISE:
        tolerance = &tolerance_precise;
        break;

    case SID_PAL_TIMER_PRIO_CLASS_LOWPOWER:
        tolerance = &tolerance_lowpower;
        break;
    }

    SID_PAL_ASSERT(tolerance);

    return tolerance;
}


static void sid_pal_timer_list_insert(struct sid_pal_timer_ctx *ctx, sid_pal_timer_t *timer, bool from_callback)
{
    SID_PAL_ASSERT(ctx && timer);
    bool reschedule_required = true;

    sid_pal_enter_critical_region();
        list_node_t * node = list_peek_head(&ctx->list);
        while (node) {
            sid_pal_timer_t * element = containerof(node, sid_pal_timer_t, node);
            if (sid_time_gt(&element->alarm, &timer->alarm)) {
                struct sid_timespec diff = element->alarm;
                sid_time_sub(&diff, &timer->alarm);
                if (!sid_time_gt(&diff, timer->tolerance)) {
                    // NB: The schedule for the timer is same, disable reschedule
                    reschedule_required = false;
                    timer->alarm = element->alarm;
                }
                list_add_before(&element->node, &timer->node);
                break;
            }
            // NB: No need for reschedule if the timer is not the first in the list
            reschedule_required = false;
            node = list_next(&ctx->list, node);
        }
        // NB: If the node is NULL there was no element with greater arm time found, insert into the tail of the list
        if (!node) {
            list_add_tail(&ctx->list, &timer->node);
        }
        if (reschedule_required && !from_callback) {
            sid_timer_resche(&timer->alarm);
        }
    sid_pal_exit_critical_region();
}


static bool sid_pal_timer_list_in_list(struct sid_pal_timer_ctx * ctx, sid_pal_timer_t * timer)
{
    SID_PAL_ASSERT(ctx && timer);
    (void)ctx;
    bool result = false;

    sid_pal_enter_critical_region();
        if (list_in_list(&timer->node)) {
            result = true;
        }
    sid_pal_exit_critical_region();

    return result;
}

static void sid_pal_timer_list_delete(struct sid_pal_timer_ctx *ctx, sid_pal_timer_t *timer)
{
    SID_PAL_ASSERT(ctx && timer);
    (void)ctx;

    sid_pal_enter_critical_region();
        if (list_in_list(&timer->node)) {
            list_delete(&timer->node);
        }
    sid_pal_exit_critical_region();
}

 static void sid_pal_timer_list_fetch(struct sid_pal_timer_ctx *ctx,
                    const struct sid_timespec *non_gt_than,
                    sid_pal_timer_t **timer)
{
    SID_PAL_ASSERT(ctx && non_gt_than && timer);

    sid_pal_timer_t * result = NULL;
    *timer = NULL;

    sid_pal_enter_critical_region();
        result = list_peek_head_type(&ctx->list, sid_pal_timer_t, node);
        if (result && !sid_time_gt(&result->alarm, non_gt_than)) {
            *timer = result;
            list_delete(&result->node);
        }
    sid_pal_exit_critical_region();
}

static void sid_pal_timer_list_get_next_schedule(struct sid_pal_timer_ctx * ctx, struct sid_timespec * schedule)
{
    SID_PAL_ASSERT(ctx && schedule);

    sid_pal_timer_t *result = NULL;
    *schedule = SID_TIME_INFINITY;

    sid_pal_enter_critical_region();
        result = list_peek_head_type(&ctx->list, sid_pal_timer_t, node);
        if (result) {
            *schedule = result->alarm;
        }
    sid_pal_exit_critical_region();
}

#if (CONFIG_TIMER_SOURCE == TIMER_SOURCE_OS)
static void sid_timer_handler(TimerHandle_t xTimer)
{
    ARG_UNUSED(xTimer);
#else
    static void sid_timer_handler(void)
    {
#endif
    struct sid_timespec handle_time;
    sid_pal_uptime_now(&handle_time);
    sid_pal_timer_event_callback_l(NULL, &handle_time);
}

_attribute_ram_code_ void timer1_irq_handler(void)
{
    if (timer_get_irq_status(FLD_TMR1_MODE_IRQ))
    {

        //HAOJIE_DBG_CHN4_HIGH;
        plic_interrupt_disable(IRQ_TIMER1);
        timer_stop(TIMER1);
        sid_timer_sleep_flag = 0;
        timer_clr_irq_status(FLD_TMR1_MODE_IRQ); //clear irq status
        sid_timer_handler();
        //HAOJIE_DBG_CHN4_LOW;
    }
}
#if (FREERTOS_ENABLE)
 PLIC_ISR_REGISTER_OS(timer1_irq_handler, IRQ_TIMER1)
#else
PLIC_ISR_REGISTER(timer1_irq_handler, IRQ_TIMER1)
#endif

static void sid_hw_timer_stop(void)
{
    #if SID_TIMER_MODE
        return;
    #endif
    plic_interrupt_disable(IRQ_TIMER1);
    timer_stop(TIMER1);
    sid_timer_sleep_flag = 0;
    blc_pm_setAppWakeupLowPower(0, 0);
}

 _attribute_ram_code_ static void hw_timer_start(uint32_t time_us)
{
//    TL_LOG_E("time_us: %d",time_us);
    sid_timer_sleep_flag = 1;
    plic_interrupt_enable(IRQ_TIMER1);
    timer_set_init_tick(TIMER1, 0);
    timer_set_cap_tick(TIMER1, time_us * sys_clk.pclk ); //500ms
    timer_set_mode(TIMER1, TIMER_MODE_SYSCLK);
    timer_set_irq_mask(FLD_TMR1_MODE_IRQ);
    timer_start(TIMER1);
}

#define HW_CODE_DELAY 1
void  sid_is_timer_running(void)
{
    sid_timer_handler();
}
#if 0
_attribute_ram_code_ void pm_irq_handler(void)
{
    sid_timer_handler();
    analog_write_reg8(0x64, 0xff);
}
#if (FREERTOS_ENABLE)
PLIC_ISR_REGISTER_OS(pm_irq_handler, IRQ_PM_IRQ)
#else
PLIC_ISR_REGISTER(pm_irq_handler, IRQ_PM_IRQ)
#endif
#endif

static void ext_32k_timer_start(uint32_t time_tick)
{
    clock_set_32k_tick(time_tick); //1s
    pm_clr_irq_status(FLD_WAKEUP_STATUS_ALL); //todo
    plic_interrupt_enable(IRQ_PM_IRQ);
}


uint32_t roll_ns = 0;

static sid_timer_get_drift(const struct sid_timespec *sid_time,const struct sid_timespec * now)
{
    struct sid_timespec drift = SID_TIME_ZERO;

}

static inline void sid_timer_start(struct sid_timespec *sid_time)
{
//    TL_LOG_D("sid_timer_start: %d %d",sid_time->tv_sec ,sid_time->tv_nsec);
    struct sid_timespec now;
#if SID_TIMER_MODE
    uint32_t dirft = 0;
    if (g_clk_32k_src == CLK_32K_RC)
    {
        dirft = NS_TO_32K_RC_COUNTER;
    }
    else
    {
        dirft = NS_TO_32K_XTAL_COUNTER;
    }
    struct sid_timespec delta = {.tv_sec = 0, .tv_nsec = dirft * 2 };
    u32 timer_duration ;
    if(sid_time_is_infinity(sid_time))
        return;
    sid_pal_uptime_now(&now);
    sid_time_add(&now,&delta) ;
////    TL_LOG_D("now: %d %d",now.tv_sec ,now.tv_nsec);
///
    if (sid_time_gt(sid_time,&now)) {
        delta.tv_nsec = HW_CODE_DELAY * dirft;
        sid_time_sub(sid_time,&delta);
    }
    else
    {
        *sid_time = now;
    }

#if (CONFIG_TIMER_SOURCE == TIMER_SOURCE_OS)
//    timer_duration = delta.tv_sec * 1000*1000 + delta.tv_nsec / 1000 + HW_CODE_DELAY;
//    hw_timer_start(timer_duration);
        sid_time_delta(&delta,sid_time,&now);
    timer_duration = pdMS_TO_TICKS(delta.tv_sec * 1000 + (delta.tv_nsec)/ 1000000);
//    roll_ns = delta.tv_nsec % (1000000000/configTICK_RATE_HZ);
    xTimerChangePeriod(g_xNextTimer,timer_duration,portMAX_DELAY);
#else
        //32k
    if (g_clk_32k_src == CLK_32K_RC)
    {
        timer_duration = sid_time->tv_sec * 32000 + sid_time->tv_nsec /31250;
    }
    else
    {
        timer_duration = sid_time->tv_sec * 32768 + sid_time->tv_nsec /30518;
    }

    ext_32k_timer_start(timer_duration);
    TL_LOG_E("timer_duration: %d",timer_duration);

#endif

#else
//    struct sid_timespec delta;
    uint32_t timer_duration ;
    if(sid_time_is_infinity(sid_time))
    {
        sid_hw_timer_stop();
        return;
    }
    uint32_t dirft = 0;
    if (g_clk_32k_src == CLK_32K_RC)
    {
        dirft = NS_TO_32K_RC_COUNTER;
    }
    else
    {
        dirft = NS_TO_32K_XTAL_COUNTER;
    }
    struct sid_timespec delta = {.tv_sec = 0, .tv_nsec = dirft * 2};
    sid_pal_uptime_now(&now);
//    sid_time_add(&now,&delta) ;
    if (sid_time_gt(sid_time,&now)) {
//        delta.tv_nsec = HW_CODE_DELAY * dirft;
//        sid_time_sub(sid_time,&delta);
        sid_time_delta(&delta,sid_time,&now);
        //32k
        timer_duration = delta.tv_sec * 1000000 + delta.tv_nsec/1000;
    }
    else
    {
        *sid_time = now;
        timer_duration = 10;  //workaroud
    }
    #if 1
    if(timer_duration <= SID_TIMER_THRELD_MAX_32K_US)
    {
        blc_pm_setAppWakeupLowPower(0,0);
        hw_timer_start(timer_duration);
//      TL_LOG_E("timer_duration %d",timer_duration);
    }
    else
    {
         u32 now_stimer = clock_time();
        if ( (timer_duration / 1000 ) < 5000 )
        {
            blc_pm_setAppWakeupLowPower(now_stimer + SYSTEM_TIMER_TICK_1US * (timer_duration - SID_TIMER_THRELD_MAX_32K_US) , 1);
            //printf("tim %d %d\r\n",now_stimer + SYSTEM_TIMER_TICK_1US * (timer_duration - SID_TIMER_THRELD_MAX_32K_MS) ,now_stimer);
        }
        else
        {
            blc_pm_setAppWakeupLowPower(now_stimer + SYSTEM_TIMER_TICK_1MS * 5000, 1);  //because maybe no ble ....
            //printf("tim %d %d\r\n",SYSTEM_TIMER_TICK_1MS * 5000,now_stimer);
        }
        sid_timer_sleep_flag = 0;
    }
    #else
        hw_timer_start(timer_duration);
    #endif

#endif
//    TL_LOG_D("sid_timer_start done %d ,%d",timer_duration,roll_ns);
}

void sid_timer_check_timer_start(void)
{
    #if SID_TIMER_MODE
        return;
    #endif

    if(sid_timer_sleep_flag)   //timer is running
       return;
    sid_pal_enter_critical_region();
    list_node_t * node = list_peek_head(&sid_pal_timer_ctx.list);
    if (node)
    {
        sid_pal_timer_t * element = containerof(node, sid_pal_timer_t, node);
        sid_timer_resche(&element->alarm);
    }
    sid_pal_exit_critical_region();

}
#ifdef CONFIG_SIDEWALK_THREAD_TIMER
static void timer_task(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    while (1) {
        xSemaphoreTake(xTimerTriggerSem, portMAX_DELAY);
        struct sid_timespec handle_time;
        sid_pal_uptime_now(&handle_time);
        sid_pal_timer_event_callback_l(NULL, &handle_time);
    }
}

#endif /* CONFIG_SIDEWALK_THREAD_TIMER */


static void sid_timer_resche(struct sid_timespec *when)
{
    if(NULL == when)
    {
        sid_hw_timer_stop();
        return ;
    }
    sid_timer_start(when);
}


sid_error_t sid_pal_timer_init(sid_pal_timer_t * timer_storage, sid_pal_timer_cb_t event_callback, void * event_callback_arg)
{
    if (!timer_storage || !event_callback) {
        return SID_ERROR_INVALID_ARGS;
    }

    sid_pal_timer_t *timer = (sid_pal_timer_t *)timer_storage;

    timer->callback     = event_callback;
    timer->callback_arg = event_callback_arg;
    timer->alarm        = SID_TIME_INFINITY;
    timer->period       = SID_TIME_INFINITY;
    list_clear_node(&timer->node);
    #ifndef CONFIG_SIDEWALK_THREAD_TIMER
    #if (CONFIG_TIMER_SOURCE == TIMER_SOURCE_OS)
    g_xNextTimer = xTimerCreateStatic("sidNext", portMAX_DELAY, pdFALSE,
                                          NULL, sid_timer_handler, &g_xNextTimerBuffer);
    #endif
    #else
    xTimerTriggerSem = xSemaphoreCreateBinaryStatic(&xTimerTriggerSemBuf);
    #endif
    return SID_ERROR_NONE;
}

sid_error_t sid_pal_timer_deinit(sid_pal_timer_t * timer_storage)
{
    if (!timer_storage) {
        return SID_ERROR_INVALID_ARGS;
    }

    sid_pal_timer_t * timer = (sid_pal_timer_t *)timer_storage;
    sid_pal_timer_list_delete(&sid_pal_timer_ctx, timer);
    timer->callback = NULL;
    timer->callback_arg = NULL;
    #if (CONFIG_TIMER_SOURCE == TIMER_SOURCE_OS)
    xTimerStop(g_xNextTimer,portMAX_DELAY);
    #endif
    return SID_ERROR_NONE;
}

sid_error_t sid_pal_timer_arm(sid_pal_timer_t * timer_storage,
                              sid_pal_timer_prio_class_t type,
                              const struct sid_timespec * when,
                              const struct sid_timespec * period)
{
    if (!timer_storage || !when) {
        return SID_ERROR_INVALID_ARGS;
    }

    sid_pal_timer_t *timer = (sid_pal_timer_t *)timer_storage;
    if (sid_pal_timer_is_armed(timer_storage)) {
        return SID_ERROR_INVALID_ARGS;
    }
    if (!period) {
        period = &SID_TIME_INFINITY;
    }

    timer->alarm    = *when;
    timer->period   = *period;
    timer->tolerance = sid_pal_timer_get_tolerance(type);
    sid_pal_timer_list_insert(&sid_pal_timer_ctx, timer, false);
//    TL_LOG_D("sid_pal_timer_arm: %d %d\n",when->tv_sec,when->tv_nsec);
    return SID_ERROR_NONE;
}

sid_error_t sid_pal_timer_cancel(sid_pal_timer_t * timer_storage)
{
    if (!timer_storage) {
        return SID_ERROR_INVALID_ARGS;
    }
    sid_pal_timer_t *timer = (sid_pal_timer_t *)timer_storage;
    sid_pal_timer_list_delete(&sid_pal_timer_ctx, timer);
    return SID_ERROR_NONE;
}

bool sid_pal_timer_is_armed(const sid_pal_timer_t * timer_storage)
{
    if (!timer_storage) {
        return false;
    }
    sid_pal_timer_t *timer = (sid_pal_timer_t *)timer_storage;
    return sid_pal_timer_list_in_list(&sid_pal_timer_ctx, timer);
}

sid_error_t sid_pal_timer_facility_init(void * arg)
{
    (void)arg;
    list_initialize(&sid_pal_timer_ctx.list);
    return SID_ERROR_NONE;
}

void sid_pal_timer_event_callback(void * arg, const struct sid_timespec * now)
{
    (void)arg;
    (void)now;
}

_attribute_ram_code_ void sid_pal_timer_event_callback_l(void * arg, const struct sid_timespec * now)
{
//    TL_LOG_D("sid_pal_timer_event_callback: %d %d %d \n",now->tv_sec,now->tv_nsec,clock_get_32k_tick());
    sid_pal_timer_t *timer = NULL;

    do {
//        // for freertos tick torrent
//        struct sid_timespec tolerance_precise2 = { .tv_sec = 0, .tv_nsec = roll_ns };
//        sid_time_add(&now,&tolerance_precise2);

        sid_pal_timer_list_fetch(&sid_pal_timer_ctx, now, &timer);
        if (!timer) {
//            TL_LOG_D("timer_NULL:\n");
            break;
        }
        if (!sid_time_is_infinity(&timer->period)) {
            sid_time_add(&timer->alarm, &timer->period);
//            TL_LOG_D("timer_list_insert(:\n");
            sid_pal_timer_list_insert(&sid_pal_timer_ctx, timer, true);
        }
//        TL_LOG_D("timer callback:\n");
        timer->callback(timer->callback_arg, (sid_pal_timer_t *)timer);
    } while (1);

    struct sid_timespec next_schedule;
    sid_pal_timer_list_get_next_schedule(&sid_pal_timer_ctx, &next_schedule);
    if(!sid_time_is_infinity(&next_schedule))
    {
        sid_timer_resche(&next_schedule);
    }
    else
    {
        sid_hw_timer_stop();
    }
}
