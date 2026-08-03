/********************************************************************************************************
 * @file    sidewalk_app.c
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
#include "sid_ble_adapter.h"
#include "stack/ble/ble.h"
#if (FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#else
#include <sid_pal_critical_region_ifc.h>
#endif
#include <sid_sdk_config.h>
#include <app.h>
//#include <sidewalk.h>
#include <app_ble_config.h>
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
#include <app_subGHz_config.h>
#endif
#include <stdbool.h>
#include <bt_app_callbacks.h>
#include <sid_api.h>
#include <sid_pal_common_ifc.h>
#include <sid_utils.h>

#include <app_mfg_config.h>
#include "app_buffer.h"
#include "app_ui.h"
#include "app_mem.h"

#define KEY1  0x01
#define KEY2  0x02
#define KEY3  0xf1
#define KEY4  0xf0

#define PARAM_UNUSED (0U)

#define MAIN_TASK_STACK_SIZE       1536
#define MSG_QUEUE_LEN 10
#define MSG_LOG_BLOCK_SIZE 80
#if (!FREERTOS_ENABLE)
#define APP_EVENTS_PER_SCHEDULE 4
#endif

enum event_type
{
      EVENT_TYPE_SIDEWALK,
      EVENT_TYPE_SEND_HELLO,
      EVENT_FACTORY_RESET,
      EVENT_TYPE_FSK_CSS_SWITCH,
      EVENT_TYPE_SET_DEVICE_PROFILE,
      EVENT_TYPE_CONNECTION_REQUEST,
};

enum app_state
{
    STATE_INIT,
    STATE_SIDEWALK_READY,
    STATE_SIDEWALK_NOT_READY,
    STATE_SIDEWALK_SECURE_CONNECTION,
};


enum evt_ind
{
     REG_IND,
     TIME_SYNC_IND,
     STATE_CONTROL,
};

struct link_status
{
    uint32_t link_mask;
    uint32_t supported_link_mode[SID_LINK_TYPE_MAX_IDX];
};

#if (!FREERTOS_ENABLE)
typedef struct app_event_queue {
    enum event_type buffer[MSG_QUEUE_LEN];
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint8_t count;
    volatile uint32_t dropped;
} app_event_queue_t;
#endif

typedef struct app_context
{
#if (FREERTOS_ENABLE)
    TaskHandle_t main_task;
    QueueHandle_t event_queue;
#else
    app_event_queue_t *event_queue;
#endif
    struct sid_handle *sidewalk_handle;
    enum app_state state;
    struct link_status link_status;
    uint8_t counter;
    bool connection_request;
} app_context_t;

/* Global mainly because button callbacks do not have a context pointer */
#if (FREERTOS_ENABLE)
static QueueHandle_t g_event_queue;
#else
static app_event_queue_t g_event_queue_buf;
static app_event_queue_t *g_event_queue;
static app_context_t g_app_context;
static struct sid_event_callbacks g_event_callbacks;
static struct sid_config g_sid_config;
static struct sid_end_device_characteristics dev_ch;
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
static sid_pal_radio_rx_packet_t g_rx;
#endif
#endif

extern char _end[];
extern uint32_t _STACK_TOP;
#define MEMORY_POOL_END    (((uint32_t)&_STACK_TOP) - 0x800)

#if BLE_APP_PM_ENABLE
void app_sleep_config(void);
#endif

void * _sbrk(ptrdiff_t incr)
{
  static uintptr_t heap_end;
  if (heap_end == 0) heap_end = (uintptr_t) _end;

  uintptr_t new_heap_end = heap_end + incr;

  if (new_heap_end > MEMORY_POOL_END) {
    errno = ENOMEM;
    return (void*) -1;
  }
  uintptr_t old_heap_end = heap_end;
  heap_end = new_heap_end;
  return (void*) old_heap_end;
}


void app_evt_state_ind(enum evt_ind ind ,uint8_t flag )
{
    #if UI_LED_ENABLE
    switch(ind)
    {
        case REG_IND:
            gpio_write(GPIO_LED_WHITE, flag);
            break;
        case TIME_SYNC_IND:
            //gpio_write(GPIO_LED_GREEN, flag);
            break;
        case STATE_CONTROL:
            gpio_write(GPIO_LED_GREEN, flag);
            break;
        default:
            break;
    }
    #endif
}

#if (!FREERTOS_ENABLE)
static void app_event_queue_init(app_event_queue_t *queue)
{
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->dropped = 0;
}

static bool app_event_queue_push(app_event_queue_t *queue, enum event_type event)
{
    bool ok = false;
    sid_pal_enter_critical_region();
    if (queue->count < MSG_QUEUE_LEN) {
        queue->buffer[queue->head] = event;
        queue->head = (uint8_t)((queue->head + 1) % MSG_QUEUE_LEN);
        queue->count++;
        ok = true;
    } else {
        queue->dropped++;
    }
    sid_pal_exit_critical_region();
    return ok;
}

static bool app_event_queue_pop(app_event_queue_t *queue, enum event_type *event)
{
    bool ok = false;
    sid_pal_enter_critical_region();
    if (queue->count > 0) {
        *event = queue->buffer[queue->tail];
        queue->tail = (uint8_t)((queue->tail + 1) % MSG_QUEUE_LEN);
        queue->count--;
        ok = true;
    }
    sid_pal_exit_critical_region();
    return ok;
}

static uint8_t app_event_queue_empty(app_event_queue_t *queue)
{
    return queue->count == 0;
}

uint8_t g_app_event_queue_empty(void)
{
    return app_event_queue_empty(g_app_context.event_queue);
}

#endif

static void queue_event(
#if (FREERTOS_ENABLE)
                        QueueHandle_t queue,
#else
                        app_event_queue_t *queue,
#endif
                        enum event_type event, bool in_isr)
{
#if (FREERTOS_ENABLE)
    if (in_isr) {
        BaseType_t task_woken = pdFALSE;
        xQueueSendFromISR(queue, &event, &task_woken);
        portYIELD_FROM_ISR(task_woken);
    }
    else {
        xQueueSend(queue, &event, 0);
    }
#else
    ARG_UNUSED(in_isr);
    if (queue != NULL) {
        app_event_queue_push(queue, event);
    }
#endif
}

static void on_sidewalk_event(bool in_isr, void *context)
{
    app_context_t *app_context = (app_context_t *)context;
    queue_event(app_context->event_queue, EVENT_TYPE_SIDEWALK, in_isr);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context)
{
    ARG_UNUSED(context);
    static uint8_t data_flag = 1;
    TL_LOG_I("received message(link_type: %d, type: %d, link_mode: %d, id: %u size %u %s)", (int)msg_desc->link_type, (int)msg_desc->type,
                             (int)msg_desc->link_mode, msg_desc->id, msg->size,msg->data);

    tlkapi_send_string_data(1, "sidewalk msg received ", msg->data, msg->size);
    char * ptr = ( char *)(msg->data);
    if(msg->size == 2 && ptr[0] == 'o' && ptr[1] == 'n')
    app_evt_state_ind(STATE_CONTROL,1);
    else
    app_evt_state_ind(STATE_CONTROL,0);
    data_flag = 1 - data_flag;
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
    ARG_UNUSED(context);
    TL_LOG_I("sent message(type: %d, id: %u)", (int)msg_desc->type, msg_desc->id);
}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context)
{
    ARG_UNUSED(context);
    TL_LOG_E("failed to send message(type: %d, id: %u), err:%d",
                  (int)msg_desc->type, msg_desc->id, (int)error);

}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
    app_context_t *app_context = (app_context_t *)context;
    TL_LOG_I("status changed: %d", (int)status->state);
    switch (status->state) {
        case SID_STATE_READY:

            app_context->state = STATE_SIDEWALK_READY;
            app_context->connection_request = false;
            break;
        case SID_STATE_NOT_READY:

            app_context->state = STATE_SIDEWALK_NOT_READY;
            break;
        case SID_STATE_ERROR:
            TL_LOG_E("sidewalk error: %d", (int)sid_get_error(app_context->sidewalk_handle));
            break;
        case SID_STATE_SECURE_CHANNEL_READY:
            app_context->state = STATE_SIDEWALK_SECURE_CONNECTION;
            break;
    }
    tlkapi_printf(1,"Registration Status = %d, Time Sync Status = %d and Link Status Mask = %x",
                 status->detail.registration_status, status->detail.time_sync_status,
                 status->detail.link_status_mask);
     if(status->detail.registration_status)
     {
         app_evt_state_ind(REG_IND,0);
     }
     else
     {
         app_evt_state_ind(REG_IND,1);
     }
     if(status->detail.time_sync_status)
     {
         app_evt_state_ind(TIME_SYNC_IND,0);
     }
     else
     {
         app_evt_state_ind(TIME_SYNC_IND,1);
     }
    app_context->link_status.link_mask= status->detail.link_status_mask;
    for (int i = 0; i < SID_LINK_TYPE_MAX_IDX; i++) {
        app_context->link_status.supported_link_mode[i] = status->detail.supported_link_modes[i];
        TL_LOG_I("Link %d Mode %x", i, status->detail.supported_link_modes[i]);
    }
}

static void on_sidewalk_factory_reset(void *context)
{
    ARG_UNUSED(context);
    TL_LOG_I("factory reset notification received from sid api");
    for(int i = 0; i < 3; i++)
    {
        #if UI_LED_ENABLE
        gpio_write(GPIO_LED_GREEN, 1);
        //gpio_write(GPIO_LED_GREEN, 1);
        gpio_write(GPIO_LED_WHITE, 1);
        delay_us(50000);
        gpio_write(GPIO_LED_GREEN, 0);
        //gpio_write(GPIO_LED_GREEN, 0);
        gpio_write(GPIO_LED_WHITE, 0);
        delay_us(50000);
        #endif
    }
    sys_reboot();
}

static void send_ping(app_context_t *app_context)
{
    if (app_context->state == STATE_SIDEWALK_READY ||
        app_context->state == STATE_SIDEWALK_SECURE_CONNECTION) {
        TL_LOG_I("sending counter update: %d", app_context->counter);
        struct sid_msg msg = {.data = (uint8_t*)&app_context->counter, .size = sizeof(uint8_t)};
        struct sid_msg_desc desc = {
            .type = SID_MSG_TYPE_NOTIFY,
            .link_type = SID_LINK_TYPE_ANY,
            .link_mode = SID_LINK_MODE_CLOUD,
        };

        if ((app_context->link_status.link_mask & SID_LINK_TYPE_1) &&
            (app_context->link_status.supported_link_mode[SID_LINK_TYPE_1_IDX] & SID_LINK_MODE_MOBILE)) {
            desc.link_mode = SID_LINK_MODE_MOBILE;
        }

        sid_error_t ret = sid_put_msg(app_context->sidewalk_handle, &msg, &desc);
        if (ret != SID_ERROR_NONE) {
            TL_LOG_E("failed queueing data, err:%d", (int) ret);

        } else {
            TL_LOG_I("queued data message id:%u", desc.id);
        }
        app_context->counter++;
    } else {
        TL_LOG_E("sidewalk is not ready yet!");

    }
}

static void factory_reset(app_context_t *context)
{
    ARG_UNUSED(context);
    sid_error_t ret = sid_set_factory_reset(context->sidewalk_handle);
    if (ret != SID_ERROR_NONE) {
        TL_LOG_E("Notification of factory reset to sid api failed!");
        sys_reboot();
    } else {
        TL_LOG_I("Wait for Sid api to notify to proceed with factory reset!");
    }
}

static void toggle_connection_request(app_context_t *context)
{
    if (context->state == STATE_SIDEWALK_READY) {
        TL_LOG_I("Sidewalk ready, operation not valid");
    } else {
        bool next = !context->connection_request;
        TL_LOG_I("%s connection request", next ? "Set" : "Clear");
        sid_error_t ret = sid_ble_bcn_connection_request(context->sidewalk_handle, next);
        if (ret == SID_ERROR_NONE) {
            context->connection_request = next;
        } else {
            TL_LOG_E("Connection request failed %d", ret);
        }
    }
}
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
static void set_device_profile(app_context_t *context, struct sid_device_profile *set_dp_cfg)
{
    struct sid_device_profile dev_cfg = {};
    sid_error_t ret = sid_option(context->sidewalk_handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                                     &dev_cfg, sizeof(dev_cfg));
    if (set_dp_cfg->unicast_params.device_profile_id != dev_cfg.unicast_params.device_profile_id
            || set_dp_cfg->unicast_params.rx_window_count != dev_cfg.unicast_params.rx_window_count
            || (set_dp_cfg->unicast_params.device_profile_id < SID_LINK3_PROFILE_A
                    && set_dp_cfg->unicast_params.unicast_window_interval.sync_rx_interval_ms
                            != dev_cfg.unicast_params.unicast_window_interval.sync_rx_interval_ms)
            || (set_dp_cfg->unicast_params.device_profile_id >= SID_LINK3_PROFILE_A
                    && set_dp_cfg->unicast_params.unicast_window_interval.async_rx_interval_ms
                            != dev_cfg.unicast_params.unicast_window_interval.async_rx_interval_ms)) {
        ret = sid_option(context->sidewalk_handle, SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
                             set_dp_cfg, sizeof(dev_cfg));
    } else {
        TL_LOG_I("device profile is already set to the desired value %d",ret);
    }
}
#endif
static int32_t init_and_start_link(app_context_t *context, struct sid_config *config, uint32_t link_mask)
{
    if (config->link_mask != link_mask) {
        sid_error_t ret = SID_ERROR_NONE;
        if (context->sidewalk_handle != NULL) {
            ret = sid_deinit(context->sidewalk_handle);
            if (ret != SID_ERROR_NONE) {
                TL_LOG_E("failed to deinitialize sidewalk, link_mask:%x, err:%d");
                goto error;
            }
        }

        struct sid_handle *sid_handle = NULL;
        config->link_mask = link_mask;
        // Initialise sidewalk
        ret = sid_init(config, &sid_handle);
        if (ret != SID_ERROR_NONE) {
            TL_LOG_E("failed to initialize sidewalk link_mask:%x, err:%d", link_mask, (int)ret);
            goto error;
        }

        // Register sidewalk handler to the application context
        context->sidewalk_handle = sid_handle;

        // Start the sidewalk stack
        ret = sid_start(sid_handle, link_mask);
        if (ret != SID_ERROR_NONE) {
            TL_LOG_E("failed to start sidewalk, link_mask:%x, err:%d", link_mask, (int)ret);
            goto error;
        }
#if CONFIG_SID_END_DEVICE_AUTO_CONN_REQ
if (link_mask == SID_LINK_TYPE_1) {
        enum sid_link_connection_policy set_policy = SID_LINK_CONNECTION_POLICY_AUTO_CONNECT;

        ret = sid_option(sid_handle, SID_OPTION_SET_LINK_CONNECTION_POLICY, &set_policy,
                   sizeof(set_policy));
        if (ret) {
            TL_LOG_E("sid option multi link manager err %d", (int)ret);
        }

        struct sid_link_auto_connect_params ac_params = {
            .link_type = SID_LINK_TYPE_1,
            .enable = true,
            .priority = 0,
            .connection_attempt_timeout_seconds = 60
        };

        ret = sid_option(sid_handle,SID_OPTION_SET_LINK_POLICY_AUTO_CONNECT_PARAMS,
                   &ac_params, sizeof(ac_params));
        if (ret) {
            TL_LOG_E("sid option multi link policy err %d", (int)ret);
        }
}else {
            enum sid_link_connection_policy set_policy =
                SID_LINK_CONNECTION_POLICY_NONE;

            ret = sid_option(sid_handle,
                             SID_OPTION_SET_LINK_CONNECTION_POLICY,
                             &set_policy,
                             sizeof(set_policy));
            if (ret) {
                TL_LOG_E("sid option set connection policy none err %d", (int)ret);
            }

            TL_LOG_I("SubGHz single link mode: disable ACM, link_mask=0x%x", link_mask);
        }

#endif
    }
    return 0;

error:
    return -1;
}



void app_radio_event_notify(sid_pal_radio_events_t evt)
{

}

void app_radio_dio_irq_handler(void)
{
    return;
}

#if (FREERTOS_ENABLE)
static void main_thread(void *context)
{
    app_context_t *app_context = (app_context_t *)context;

    struct sid_event_callbacks event_callbacks = {
        .context = app_context,
        .on_event = on_sidewalk_event, /* Called from ISR context */
        .on_msg_received = on_sidewalk_msg_received, /* Called from sid_process() */
        .on_msg_sent = on_sidewalk_msg_sent,  /* Called from sid_process() */
        .on_send_error = on_sidewalk_send_error, /* Called from sid_process() */
        .on_status_changed = on_sidewalk_status_changed, /* Called from sid_process() */
        .on_factory_reset = on_sidewalk_factory_reset, /* Called from sid_process */
    };

    struct sid_end_device_characteristics dev_ch = {
        .type = SID_END_DEVICE_TYPE_STATIC,
        .power_type = SID_END_DEVICE_POWERED_BY_BATTERY_AND_LINE_POWER,
        .qualification_id = 0x0005,
    };

    struct sid_config config = {
        .link_mask = 0,
        .dev_ch = dev_ch,
        .callbacks = &event_callbacks,
        .link_config = app_get_ble_config(),
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
        .sub_ghz_link_config = app_get_sub_ghz_config(),
#endif
    };

    if (init_and_start_link(app_context, &config, SID_LINK_TYPE_1) != 0) {
            goto error;
    }
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
    sid_pal_radio_rx_packet_t         g_rx;
    sid_pal_radio_init(app_radio_event_notify, app_radio_dio_irq_handler, &(g_rx));
    if(0 !=sid_pal_radio_sleep(UINT32_MAX)) //save power
    {
        TL_LOG_E("sid_pal_radio_sleep fail");
    }
#endif
    app_context->state = STATE_SIDEWALK_NOT_READY;
    app_context->connection_request = false;
    while (1) {
        enum event_type event;
        if (xQueueReceive(app_context->event_queue, &event, portMAX_DELAY) == pdTRUE) {
            switch (event) {
                case EVENT_TYPE_SIDEWALK:
                    sid_process(app_context->sidewalk_handle );
                    break;
                case EVENT_TYPE_SEND_HELLO:
                    send_ping(app_context);
                    break;
                case EVENT_FACTORY_RESET:
                    factory_reset(app_context);
                    break;
                case EVENT_TYPE_CONNECTION_REQUEST:
                    toggle_connection_request(app_context);
                    break;
                #ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
                case EVENT_TYPE_FSK_CSS_SWITCH:
                    if (config.link_mask == SID_LINK_TYPE_1 || config.link_mask == SID_LINK_TYPE_2) {
                        if (init_and_start_link(app_context, &config, SID_LINK_TYPE_3) != 0) {
                            goto error;
                        }
                        TL_LOG_I("app: Switching to CSS...");
                     } else if (config.link_mask == SID_LINK_TYPE_3) {
                         if (init_and_start_link(app_context, &config, SID_LINK_TYPE_2) != 0) {
                             goto error;
                         }
                         TL_LOG_I("app: Switching to FSK...");
                     } else {
                         TL_LOG_W("app: FSK/CSS switch can not be performed");
                     }

                    break;

                case EVENT_TYPE_SET_DEVICE_PROFILE: {
                    struct sid_device_profile set_dp_cfg = {};
                    struct sid_device_profile dev_cfg = {};
                    if (config.link_mask != SID_LINK_TYPE_3) {
                        if (init_and_start_link(app_context, &config, SID_LINK_TYPE_3) != 0) {
                            goto error;
                        }
                    }
                    dev_cfg.unicast_params.device_profile_id = SID_LINK3_PROFILE_B;
                    sid_option(app_context->sidewalk_handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE, &dev_cfg, sizeof(dev_cfg));
                    set_dp_cfg = dev_cfg;
                    if (dev_cfg.unicast_params.device_profile_id == SID_LINK3_PROFILE_A) {
                                set_dp_cfg.unicast_params.device_profile_id = SID_LINK3_PROFILE_B;
                                set_dp_cfg.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE;
                                set_dp_cfg.unicast_params.unicast_window_interval.async_rx_interval_ms = SID_LINK3_RX_WINDOW_SEPARATION_3;
                    } else if (dev_cfg.unicast_params.device_profile_id == SID_LINK3_PROFILE_B) {
                                set_dp_cfg.unicast_params.device_profile_id = SID_LINK3_PROFILE_A;
                                set_dp_cfg.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_2;
                    }
                    TL_LOG_I("changing from profile : %d -> %d, rx_interval %d",
                                  dev_cfg.unicast_params.device_profile_id,
                                  set_dp_cfg.unicast_params.device_profile_id,
                                  set_dp_cfg.unicast_params.unicast_window_interval.async_rx_interval_ms);
                    set_device_profile(app_context, &set_dp_cfg);
                    break;
                    }
                    #endif
                }
           }
    }

error:
    if (app_context->sidewalk_handle  != NULL) {
        sid_stop(app_context->sidewalk_handle , config.link_mask);
        sid_deinit(app_context->sidewalk_handle );
        app_context->sidewalk_handle = NULL;
    }
    fflush(NULL);
    sys_reboot();
    vTaskDelete(NULL);
}
#endif /* FREERTOS_ENABLE */


void Portble_btn_press(u8 key)
{
    if(KEY1 == key)
    {
        queue_event(g_event_queue, EVENT_TYPE_FSK_CSS_SWITCH, true);
    }
    else if(KEY2 == key)
    {
        queue_event(g_event_queue, EVENT_TYPE_SET_DEVICE_PROFILE, true);
    }
    else    if(KEY3 == key)
    {
        queue_event(g_event_queue, EVENT_TYPE_CONNECTION_REQUEST, true);
    }
    else
    {
        queue_event(g_event_queue,  EVENT_TYPE_SEND_HELLO, true);
    }
}

void Portble_btn_d_press(u8 key)
{
    ARG_UNUSED(key);
    queue_event(g_event_queue, EVENT_FACTORY_RESET, true);
}
void Portble_btn_l_press(u8 key)
{
    ARG_UNUSED(key);
    queue_event(g_event_queue, EVENT_TYPE_SEND_HELLO, true);
}


#if (FREERTOS_ENABLE)
int app_start(void)
{
    #if BLE_APP_PM_ENABLE
    app_sleep_config();
    #endif
    platform_parameters_t platform_parameters = {
            .mfg_store_region.addr_start = sid_mfg_get_start_addr(),
            .mfg_store_region.addr_end = sid_mfg_get_end_addr(),
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
            .platform_init_parameters.radio_cfg = (radio_sx126x_device_config_t*)get_radio_cfg(),
#endif
    };

    sid_error_t ret_code = sid_platform_init(&platform_parameters);
    if (ret_code != SID_ERROR_NONE) {
        TL_LOG_E("Sidewalk Platform Init err: %d", ret_code);
         configASSERT(0);
    }
    TL_LOG_D("Sidewalk Platform Init done");
    g_event_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(enum event_type));
    if (g_event_queue == NULL) {
        TL_LOG_E("xQueueCreate init  err");
         configASSERT(0);
    }

    static app_context_t app_context = {
#if (FREERTOS_ENABLE)
        .main_task = NULL,
#endif
        .event_queue = NULL,
        .sidewalk_handle = NULL,
        .state = STATE_INIT,
    };

    app_context.event_queue = g_event_queue;

    if (pdPASS != xTaskCreate(main_thread, "sidewalk", MAIN_TASK_STACK_SIZE, &app_context, CONFIG_SIDEWALK_THREAD_PRIORITY, &app_context.main_task)) {
        TL_LOG_E("sidewalk xTaskCreate init  err");
         configASSERT(0);
    }
    return 0;
}

#else /* !FREERTOS_ENABLE */

static void app_prepare_config(app_context_t *app_context)
{
    dev_ch = (struct sid_end_device_characteristics){
        .type = SID_END_DEVICE_TYPE_STATIC,
        .power_type = SID_END_DEVICE_POWERED_BY_BATTERY_AND_LINE_POWER,
        .qualification_id = 0x0005,
    };

    g_event_callbacks = (struct sid_event_callbacks) {
        .context = app_context,
        .on_event = on_sidewalk_event,
        .on_msg_received = on_sidewalk_msg_received,
        .on_msg_sent = on_sidewalk_msg_sent,
        .on_send_error = on_sidewalk_send_error,
        .on_status_changed = on_sidewalk_status_changed,
        .on_factory_reset = on_sidewalk_factory_reset,
    };

    g_sid_config = (struct sid_config) {
        .link_mask = 0,
        .dev_ch = dev_ch,
        .callbacks = &g_event_callbacks,
        .link_config = app_get_ble_config(),
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
        .sub_ghz_link_config = app_get_sub_ghz_config(),
#endif
    };
}

static int app_process_event(enum event_type event)
{
    switch (event) {
        case EVENT_TYPE_SIDEWALK:
            sid_process(g_app_context.sidewalk_handle);
            break;
        case EVENT_TYPE_SEND_HELLO:
            send_ping(&g_app_context);
            break;
        case EVENT_FACTORY_RESET:
            factory_reset(&g_app_context);
            break;
        case EVENT_TYPE_CONNECTION_REQUEST:
            toggle_connection_request(&g_app_context);
            break;
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
        case EVENT_TYPE_FSK_CSS_SWITCH:
            if (g_sid_config.link_mask == SID_LINK_TYPE_1 || g_sid_config.link_mask == SID_LINK_TYPE_2) {
                if (init_and_start_link(&g_app_context, &g_sid_config, SID_LINK_TYPE_3) != 0) {
                    return -1;
                }
                TL_LOG_I("app: Switching to CSS...");
            } else if (g_sid_config.link_mask == SID_LINK_TYPE_3) {
                if (init_and_start_link(&g_app_context, &g_sid_config, SID_LINK_TYPE_2) != 0) {
                    return -1;
                }
                TL_LOG_I("app: Switching to FSK...");
            } else {
                TL_LOG_W("app: FSK/CSS switch can not be performed");
            }
            break;

        case EVENT_TYPE_SET_DEVICE_PROFILE: {
            struct sid_device_profile set_dp_cfg = {};
            struct sid_device_profile dev_cfg = {};
            if (g_sid_config.link_mask != SID_LINK_TYPE_3) {
                if (init_and_start_link(&g_app_context, &g_sid_config, SID_LINK_TYPE_3) != 0) {
                    return -1;
                }
            }
            dev_cfg.unicast_params.device_profile_id = SID_LINK3_PROFILE_B;
            sid_option(g_app_context.sidewalk_handle, SID_OPTION_900MHZ_GET_DEVICE_PROFILE, &dev_cfg, sizeof(dev_cfg));
            set_dp_cfg = dev_cfg;
            if (dev_cfg.unicast_params.device_profile_id == SID_LINK3_PROFILE_A) {
                set_dp_cfg.unicast_params.device_profile_id = SID_LINK3_PROFILE_B;
                set_dp_cfg.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_INFINITE;
                set_dp_cfg.unicast_params.unicast_window_interval.async_rx_interval_ms = SID_LINK3_RX_WINDOW_SEPARATION_3;
            } else if (dev_cfg.unicast_params.device_profile_id == SID_LINK3_PROFILE_B) {
                set_dp_cfg.unicast_params.device_profile_id = SID_LINK3_PROFILE_A;
                set_dp_cfg.unicast_params.rx_window_count = SID_RX_WINDOW_CNT_2;
            }
            TL_LOG_I("changing from profile : %d -> %d, rx_interval %d",
                     dev_cfg.unicast_params.device_profile_id,
                     set_dp_cfg.unicast_params.device_profile_id,
                     set_dp_cfg.unicast_params.unicast_window_interval.async_rx_interval_ms);
            set_device_profile(&g_app_context, &set_dp_cfg);
            break;
        }
#endif
        default:
            break;
    }
    return 0;
}

int app_sidewalk_init(void)
{
#if BLE_APP_PM_ENABLE
    app_sleep_config();
#endif
    platform_parameters_t platform_parameters = {
            .mfg_store_region.addr_start = sid_mfg_get_start_addr(),
            .mfg_store_region.addr_end = sid_mfg_get_end_addr(),
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
            .platform_init_parameters.radio_cfg = (radio_sx126x_device_config_t*)get_radio_cfg(),
#endif
    };

    sid_error_t ret_code = sid_platform_init(&platform_parameters);
    if (ret_code != SID_ERROR_NONE) {
        TL_LOG_E("Sidewalk Platform Init err: %d", ret_code);
        return -1;
    }
    TL_LOG_I("Sidewalk Platform Init done");

    app_event_queue_init(&g_event_queue_buf);
    g_event_queue = &g_event_queue_buf;

    g_app_context = (app_context_t) {
        .event_queue = g_event_queue,
        .sidewalk_handle = NULL,
        .state = STATE_INIT,
    };
    app_prepare_config(&g_app_context);

    uint32_t ret = init_and_start_link(&g_app_context, &g_sid_config, SID_LINK_TYPE_1);

    if (ret != 0) {
        TL_LOG_E("init_and_start_link err: %d", ret);
        return -1;
    }
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
    sid_pal_radio_init(app_radio_event_notify, app_radio_dio_irq_handler, &g_rx);
    if (0 != sid_pal_radio_sleep(UINT32_MAX)) {
        TL_LOG_E("sid_pal_radio_sleep fail");
    }
#endif
    g_app_context.state = STATE_SIDEWALK_NOT_READY;
    g_app_context.connection_request = false;
    return 0;
}

void app_sidewalk_sch(void)
{
    enum event_type event;
    uint8_t processed = 0;
    
    while (processed < APP_EVENTS_PER_SCHEDULE && app_event_queue_pop(g_event_queue, &event)) {
        //TL_LOG_I("sid event %d", event);
        if (app_process_event(event) != 0) {
            TL_LOG_E("sid event err");
            if (g_app_context.sidewalk_handle != NULL) {
                sid_stop(g_app_context.sidewalk_handle, g_sid_config.link_mask);
                sid_deinit(g_app_context.sidewalk_handle);
                g_app_context.sidewalk_handle = NULL;
            }
            sys_reboot();
        }
        processed++;
    }
}

#endif /* FREERTOS_ENABLE */

#if BLE_APP_PM_ENABLE
extern void app_sid_subg_sleep_enter(u8 e, u8 *p, int n);
extern void app_sid_subg_wakeup(u8 e, u8 *p, int n);
#if (FREERTOS_ENABLE)
extern void proc_keyboardSupend (u8 e, u8 *p, int n);
#else
extern void proc_keyboard(u8 e, u8 *p, int n);
#endif
#if (UI_BUTTON_ENABLE )
extern void app_set_button_wakeup(u8 e, u8 *p, int n);
#endif

_attribute_ram_code_ void app_sid_sleep_enter(u8 e, u8 *p, int n)
{
    (void)e;
    (void)p;
    (void)n;
    #if (UI_KEYBOARD_ENABLE )
    app_set_kb_wakeup(e,p,n);
    #endif
    #if (UI_BUTTON_ENABLE )
    app_set_button_wakeup(e,p,n);
    #endif
    #ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
    app_sid_subg_sleep_enter(e,p,n);
    #endif
}

void app_sid_wakeup(u8 e, u8 *p, int n)
{
    #if (UI_KEYBOARD_ENABLE || UI_BUTTON_ENABLE)
    #if (FREERTOS_ENABLE)
    proc_keyboardSupend(e,p,n);
    #else
#if (UI_KEYBOARD_ENABLE)
    proc_keyboard(e,p,n);
#endif
#if (UI_BUTTON_ENABLE)
    proc_button(e,p,n);
#endif
    #endif
    #endif
    #ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
    //app_sid_subg_wakeup(e,p,n);
    #endif
}

void app_sleep_config(void)
{
    blc_ll_registerTelinkControllerEventCallback(BLT_EV_FLAG_SLEEP_ENTER, &app_sid_sleep_enter);
    blc_ll_registerTelinkControllerEventCallback(BLT_EV_FLAG_GPIO_EARLY_WAKEUP, &app_sid_wakeup);
    blc_ll_registerTelinkControllerEventCallback(BLT_EV_FLAG_SUSPEND_EXIT, &app_sid_subg_wakeup);
}

#endif

