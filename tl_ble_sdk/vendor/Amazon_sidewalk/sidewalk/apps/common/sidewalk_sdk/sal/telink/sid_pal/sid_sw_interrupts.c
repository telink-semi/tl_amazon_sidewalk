/********************************************************************************************************
 * @file    sid_sw_interrupts.c
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
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "sid_ble_adapter.h"
#include <sid_pal_swi_ifc.h>
#if (FREERTOS_ENABLE)
    #include "tlk_riscv.h"
    #include <FreeRTOS.h>
    #include <task.h>
    #include <timers.h>
    #include <queue.h>
    #include <semphr.h>
    #include <event_groups.h>
    #include "app_freertos.h"


#ifndef CONFIG_SIDEWALK_SWI_PRIORITY
#error "CONFIG_SIDEWALK_SWI_PRIORITY must be defined"
#endif


#ifndef SIDEWALK_SWI_STACK_SIZE
#define SIDEWALK_SWI_STACK_SIZE 1024
#endif
#ifndef SIDEWALK_SWI_TASK_PRIO
#define SIDEWALK_SWI_TASK_PRIO  CONFIG_SIDEWALK_SWI_PRIORITY
#endif


_attribute_ble_data_retention_ static sid_pal_swi_cb_t   swi_cb   = NULL;
_attribute_ble_data_retention_ static bool               is_init  = false;
_attribute_ble_data_retention_ static TaskHandle_t hswiTask = NULL;
_attribute_ble_data_retention_ static SemaphoreHandle_t swi_sem = NULL;

/*-----------------------------------------------------------*/
static void swi_task(void *pv)
{
    (void)pv;
    while (1) {

        if (xSemaphoreTake(swi_sem, portMAX_DELAY) == pdTRUE) {
            if (swi_cb) {
                swi_cb();
            }
        }
    }
}

/*-----------------------------------------------------------*/
sid_error_t sid_pal_swi_init(void)
{
    if (is_init) {
        return SID_ERROR_NONE;
    }

    swi_sem = xSemaphoreCreateBinary();
    if (swi_sem == NULL) {
        return SID_ERROR_GENERIC;
    }

    int ret = xTaskCreate(swi_task, "sid_swi", SIDEWALK_SWI_STACK_SIZE, (void *)0, SIDEWALK_SWI_TASK_PRIO, &hswiTask);
    configASSERT(ret == pdPASS);

    TL_LOG_D("sid_pal_swi_init done");
    is_init = true;
    return SID_ERROR_NONE;
}

sid_error_t sid_pal_swi_deinit(void)
{
    if (!is_init) {
        return SID_ERROR_NONE;
    }
    sid_pal_swi_stop();


    vTaskDelete(hswiTask);
    vSemaphoreDelete(swi_sem);
    swi_sem = NULL;
    is_init = false;
    return SID_ERROR_NONE;
}

sid_error_t sid_pal_swi_start(sid_pal_swi_cb_t event_callback)
{
    if (!event_callback) {
        return SID_ERROR_NULL_POINTER;
    }
    swi_cb = event_callback;
    return SID_ERROR_NONE;
}

sid_error_t sid_pal_swi_stop(void)
{
    swi_cb = NULL;
    return SID_ERROR_NONE;
}

sid_error_t sid_pal_swi_trigger(void)
{
    if (!(is_init && swi_sem)) {
        return SID_ERROR_INVALID_STATE;
    }
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (xPortIsInsideInterrupt()) {
        xSemaphoreGiveFromISR(swi_sem, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    } else {
        xSemaphoreGive(swi_sem);
    }
    return SID_ERROR_NONE;
}
#else

_attribute_ble_data_retention_ static sid_pal_swi_cb_t   swi_cb   = NULL;
_attribute_ble_data_retention_ static bool               is_init  = false;

_attribute_ram_code_ mtime_irq_handler(void)
{

}

_attribute_ram_code_ void mswi_irq_handler(void)
{
    if (swi_cb) {
        swi_cb();
    }
}

_attribute_ram_code_noinline_ __attribute__((interrupt("machine"), aligned(4))) void trap_entry(void)
{
    long mcause   = read_csr(NDS_MCAUSE);
    long mepc     = 0;
    long mstatus  = 0;
    long mxstatus = 0;

    if (g_plic_preempt_en) {
        mepc     = read_csr(NDS_MEPC);
        mstatus  = read_csr(NDS_MSTATUS);
        mxstatus = read_csr(NDS_MXSTATUS);
    }

    if ((mcause & 0x80000000UL) && ((mcause & 0x7FFFFFFFUL) == 7)) /* machine timer interrupt */
    {
        if (g_plic_preempt_en) {
            /* before enable global interrupt,disable the timer interrupt to prevent re-entry */
            core_mie_disable(FLD_MIE_MTIE);
            set_csr(NDS_MSTATUS, FLD_MSTATUS_MIE);
        }

        mtime_irq_handler();

        if (g_plic_preempt_en) {
            clear_csr(NDS_MSTATUS, FLD_MSTATUS_MIE);
            /* re-enable the timer interrupt. */
            core_mie_enable(FLD_MIE_MTIE);
        }
    } else if ((mcause & 0x80000000UL) && ((mcause & 0x7FFFFFFFUL) == 3)) /* machine software interrupt */
    {
        plic_sw_interrupt_claim();
        /* if support interrupt nest,enable global interrupt */
        if (g_plic_preempt_en) {
            set_csr(NDS_MSTATUS, FLD_MSTATUS_MIE);
        }

        mswi_irq_handler();

        if (g_plic_preempt_en) {
            clear_csr(NDS_MSTATUS, FLD_MSTATUS_MIE);
        }

        plic_sw_interrupt_complete();
    } else /* unhandled Trap */
    {
        except_handler();
    }

    if (g_plic_preempt_en) {
        write_csr(NDS_MSTATUS, mstatus);
        write_csr(NDS_MEPC, mepc);
        write_csr(NDS_MXSTATUS, mxstatus);
    }
}
/*-----------------------------------------------------------*/
sid_error_t sid_pal_swi_init(void)
{
    if (is_init) {
        return SID_ERROR_NONE;
    }
    is_init = true;
    /* 2: enable software interrupt */
    core_mie_enable(FLD_MIE_MSIE);
    /* 3: plic_sw interrupt enable */
    plic_sw_interrupt_enable();
    TL_LOG_D("sid_pal_swi_init done");
    return SID_ERROR_NONE;
}

sid_error_t sid_pal_swi_deinit(void)
{
    if (!is_init) {
        return SID_ERROR_NONE;
    }
    sid_pal_swi_stop();
    /* 2: enable software interrupt */
    core_mie_disable(FLD_MIE_MSIE);
    /* 3: plic_sw interrupt enable */
    plic_sw_interrupt_disable();
    is_init = false;
    return SID_ERROR_NONE;
}

sid_error_t sid_pal_swi_start(sid_pal_swi_cb_t event_callback)
{
    if (!event_callback) {
        return SID_ERROR_NULL_POINTER;
    }
    swi_cb = event_callback;
    return SID_ERROR_NONE;
}

sid_error_t sid_pal_swi_stop(void)
{
    swi_cb = NULL;
    return SID_ERROR_NONE;
}

sid_error_t sid_pal_swi_trigger(void)
{
    if (!(is_init )) {
        return SID_ERROR_INVALID_STATE;
    }
    plic_sw_set_pending(); /* trigger swi */
    return SID_ERROR_NONE;
}

#endif
