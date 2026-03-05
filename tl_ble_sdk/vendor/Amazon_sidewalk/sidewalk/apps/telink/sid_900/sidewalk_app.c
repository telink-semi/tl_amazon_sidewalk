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
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <sid_sdk_config.h>
#include <app.h>
//#include <sidewalk.h>
#include <app_ble_config.h>
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
#include <app_subGHz_config.h>
#endif
#include <sid_hal_reset_ifc.h>
//#include <sid_hal_memory_ifc.h>
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
#define KEY2  0x2
#define KEY3  0xf1
#define KEY4  0xf0

#define PARAM_UNUSED (0U)

#define MAIN_TASK_STACK_SIZE        (4096 / sizeof(configSTACK_DEPTH_TYPE))
#define MSG_QUEUE_LEN 10
#define MSG_LOG_BLOCK_SIZE 80

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

typedef struct app_context
{
    TaskHandle_t main_task;
    QueueHandle_t event_queue;
    struct sid_handle *sidewalk_handle;
    enum app_state state;
    struct link_status link_status;
    uint8_t counter;
    bool connection_request;
} app_context_t;

/* Global mainly because bsp_evt_handler does not have a context pointer */
static QueueHandle_t g_event_queue;

extern char _end[];

void * _sbrk(ptrdiff_t incr)
{
  static uintptr_t heap_end;
  if (heap_end == 0) heap_end = (uintptr_t) _end;

  uintptr_t new_heap_end = heap_end + incr;

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
            gpio_write(GPIO_LED_RED, flag);
            break;
        case TIME_SYNC_IND:
            //gpio_write(GPIO_LED_GREEN, flag);
            break;
        case STATE_CONTROL:
            gpio_write(GPIO_LED_BLUE, flag);
                break;
        default:
            break;
    }
    #endif
}

static void queue_event(QueueHandle_t queue, enum event_type event, bool in_isr)
{
    if (in_isr) {
        BaseType_t task_woken = pdFALSE;
        xQueueSendFromISR(queue, &event, &task_woken);
        portYIELD_FROM_ISR(task_woken);
    }
    else {
        xQueueSend(queue, &event, 0);
    }
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
    TL_LOG_I("received message(type: %d, link_mode: %d, id: %u size %u %s)", (int)msg_desc->type,
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
    TL_LOG_I("Registration Status = %d, Time Sync Status = %d and Link Status Mask = %x",
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
        gpio_write(GPIO_LED_BLUE, 1);
        //gpio_write(GPIO_LED_GREEN, 1);
        gpio_write(GPIO_LED_RED, 1);
        delay_us(50000);
        gpio_write(GPIO_LED_BLUE, 0);
        //gpio_write(GPIO_LED_GREEN, 0);
        gpio_write(GPIO_LED_RED, 0);
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

        enum sid_link_connection_policy set_policy =
            SID_LINK_CONNECTION_POLICY_AUTO_CONNECT;

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

#endif
    }
    return 0;

error:
    context->sidewalk_handle = NULL;
    config->link_mask = 0;
    return -1;
}



void app_radio_event_notify(sid_pal_radio_events_t evt)
{

}

void app_radio_dio_irq_handler(void)
{
    return;
}

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

    struct sid_handle *sid_handle = NULL;
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
            TL_LOG_D("sid event %d", event);
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
    if (sid_handle != NULL) {
        sid_stop(sid_handle, SID_LINK_TYPE_1);
        sid_deinit(sid_handle);
        app_context->sidewalk_handle = NULL;
    }

    vTaskDelete(NULL);
}



void Portble_btn_press(u8 key)
{
    if(KEY1 == key)
    {
        queue_event(g_event_queue, EVENT_TYPE_SEND_HELLO, true);
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
        queue_event(g_event_queue, EVENT_TYPE_FSK_CSS_SWITCH, true);
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
    queue_event(g_event_queue, EVENT_FACTORY_RESET, true);
}


int app_start(void)
{
    // init the buffer to malloc/free
//    memset(portble_non_ret_buf, 0, PORTBLE_NON_RET_BUF_SIZE);
//    app_initialNonRetentionBuffer(portble_non_ret_buf, PORTBLE_NON_RET_BUF_SIZE);
    #if BLE_APP_PM_ENABLE
    void app_sleep_config(void);
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
        .event_queue = NULL,
        .main_task = NULL,
        .sidewalk_handle = NULL,
        .state = STATE_INIT,
    };

    app_context.event_queue = g_event_queue;

    if (pdPASS != xTaskCreate(main_thread, "sidewalk", MAIN_TASK_STACK_SIZE, &app_context, CONFIG_SIDEWALK_THREAD_PRIORITY, &app_context.main_task)) {
        TL_LOG_E("sidewalk xTaskCreate init  err");
         configASSERT(0);
    }

}

#if BLE_APP_PM_ENABLE
extern void app_sid_set_wakeup_pin(void);
extern void app_set_kb_wakeup(u8 e, u8 *p, int n);
extern void proc_keyboard(u8 e, u8 *p, int n);
extern void proc_keyboardSupend (u8 e, u8 *p, int n);


_attribute_ram_code_ void app_sid_sleep_enter(u8 e, u8 *p, int n)
{
    (void)e;
    (void)p;
    (void)n;
    app_set_kb_wakeup(e,p,n);
    app_sid_set_wakeup_pin();
}

void app_sid_wakeup(u8 e, u8 *p, int n)
{
    #if (FREERTOS_ENABLE)
    proc_keyboardSupend(e,p,n);
    #else
    proc_keyboard(e,p,n);
    #endif
}

void app_sleep_config(void)
{
    blc_ll_registerTelinkControllerEventCallback(BLT_EV_FLAG_SLEEP_ENTER, &app_sid_sleep_enter);
    blc_ll_registerTelinkControllerEventCallback(BLT_EV_FLAG_GPIO_EARLY_WAKEUP, &app_sid_wakeup);
}

#endif

