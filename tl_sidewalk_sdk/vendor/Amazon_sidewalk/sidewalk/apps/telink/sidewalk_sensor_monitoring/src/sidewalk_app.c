/********************************************************************************************************
 * @file
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
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <sid_sdk_config.h>
#include <app.h>
#include "timers.h"

#include <app_ble_config.h>
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
#include <app_subGHz_config.h>
#endif
#include <sid_hal_reset_ifc.h>
#include <stdbool.h>
#include <bt_app_callbacks.h>
#include <sid_api.h>
#include <sid_pal_common_ifc.h>
#include <app_mfg_config.h>
#include "app_ui.h"

#include "sidewalk/sidewalk_worker.h"
#include "sidewalk/message_encoder.h"
#include <sid_clock_ifc.h>
#include <sid_asd_cli.h>
#include <sid_config_cli.h>
#include <sid_qa.h>
#include <sid_device_information.h>
#include <sid_utils.h>
#include "sid_pal_uptime_ifc.h"

#include "app_mem.h"
#include "app_buffer.h"

#include "sensor_monitoring/app_err.h"
#include "tl_wrappers.h"
#include "sensor_monitoring/app_leds.h"
#include "sensor_monitoring/app_buttons.h"
#include "sensor_monitoring/app_sensor.h"
#include "sensor_monitoring/st7789.h"
#include "sidewalk/message_encoder.h"

#define NOTIFY_TIMER_DURATION_MS (500)
#define SENSOR_UPDATE_INTERVAL   (30)  // 15s
#define APP_WT_REBOOT_INTERVAL   (240) // 2min.
#define APP_DUMMY_SENSOR_DATA    (42)
#define MAX_TEXT_MESSAGE_LEN     (128)

#define RAT_SELLECTOR_DELAY      (20) //10s

static void notify_timer_cb(TimerHandle_t th);

_attribute_ble_data_retention_ bool triggerReportAfterConnection = false;

typedef enum {
    APP_STATE_NOT_CONNECTED,
    APP_STATE_CONNECTED,
    APP_STATE_RUNNING
} app_state_t;

_attribute_ble_data_retention_ app_state_t APPstate = APP_STATE_NOT_CONNECTED;
_attribute_ble_data_retention_ uint16_t APP_WDT = 0;
_attribute_ble_data_retention_ uint32_t currentLinkType = 0;

_attribute_ble_data_retention_ bool noSleep = true;

void app_set_current_link_type(uint32_t linkTypeToSet) {
    currentLinkType = linkTypeToSet;
}

uint32_t app_get_current_link_type(void) {
    return currentLinkType;
}

static uint32_t get_uptime(void) {
    struct sid_timespec now;
    sid_pal_uptime_now(&now);

    return now.tv_sec;
}

void app_send_capabilities(void) {
    int retVal;
    // Prepare message
    TL_LOG_I("[APP] Notyffying Capabilities");
    static app_message_to_encode_t appMsg;
    static uint8_t LEDids[APP_NUMBER_OF_LEDS]    = {0};
    static uint8_t BTNids[APP_NUMBER_OF_BUTTONS] = {0};

    app_LED_get_IDs(LEDids);
    app_get_button_IDs(BTNids);

    appMsg.messageType = APP_MESSAGE_CAPABILITIES;
    appMsg.linkType = app_get_current_link_type();

    appMsg.data.capabilities.deviceCapabilities.link_type     = app_get_current_link_type();
    appMsg.data.capabilities.deviceCapabilities.temp_sensor   = SID_DEMO_TEMPERATURE_SENSOR_UNITS_CELSIUS;
    appMsg.data.capabilities.deviceCapabilities.button_id_arr = BTNids;
    appMsg.data.capabilities.deviceCapabilities.num_buttons   = APP_NUMBER_OF_BUTTONS;
    appMsg.data.capabilities.deviceCapabilities.led_id_arr    = LEDids;
    appMsg.data.capabilities.deviceCapabilities.num_leds      = APP_NUMBER_OF_LEDS;

    sidewalk_worker_take_job(SIDEWALK_WORKER_DO_SEND_MESSAGE, &appMsg);
}

void app_send_button_report(void) {
       int retVal;
        // Read button state
        static app_message_to_encode_t appMsg;
        static uint8_t button_arr[APP_NUMBER_OF_BUTTONS] = {0};
        uint8_t num_buttons                 = 0;
        num_buttons = app_get_pressed_buttons(button_arr);
        TL_LOG_I("[APP] ##### Sending button report: num_buttons:%d ba={%d, %d}", num_buttons, button_arr[0], button_arr[1]);

        appMsg.data.buttonsReport.buttonsNotification.gps_time_in_seconds                = get_uptime(),
        appMsg.data.buttonsReport.buttonsNotification.link_type                          = app_get_current_link_type(),
        appMsg.data.buttonsReport.buttonsNotification.temp_sensor                        = SID_DEMO_TEMPERATURE_SENSOR_NOT_SUPPORTED,
        appMsg.data.buttonsReport.buttonsNotification.button_action_notify.action_resp   = SID_DEMO_ACTION_BUTTON_PRESSED,
        appMsg.data.buttonsReport.buttonsNotification.button_action_notify.button_id_arr = button_arr,
        appMsg.data.buttonsReport.buttonsNotification.button_action_notify.num_buttons   = num_buttons,

        appMsg.messageType = APP_MESSAGE_BUTTONS_REPORT;
        appMsg.linkType = app_get_current_link_type();

        sidewalk_worker_take_job(SIDEWALK_WORKER_DO_SEND_MESSAGE, &appMsg);

        app_consume_all_buttons();
        TL_LOG_D("[APP] Button report sent");
}

void app_send_sensors_report(void) {
        static app_message_to_encode_t appMsg;
        int retVal;
        int16_t temp = 0;
        retVal = app_sensor_temperature_get(&temp);
        if (retVal) {
            TL_LOG_D("[APP] Temperature get err %d, use dummy value: %d", retVal, APP_DUMMY_SENSOR_DATA);
            temp = APP_DUMMY_SENSOR_DATA;
        }

        TL_LOG_I("# # # Notyffying Sensors");
        // Prepare message
        appMsg.data.sensorsReport.sensorsNotification.gps_time_in_seconds              = get_uptime(),
        appMsg.data.sensorsReport.sensorsNotification.link_type                        = app_get_current_link_type(),
        appMsg.data.sensorsReport.sensorsNotification.temp_sensor                      = SID_DEMO_TEMPERATURE_SENSOR_UNITS_CELSIUS,
        appMsg.data.sensorsReport.sensorsNotification.temperature                      = temp,
        appMsg.data.sensorsReport.sensorsNotification.button_action_notify.action_resp = SID_DEMO_ACTION_BUTTON_NOT_PRESSED,

#if(APP_REPORT_IMU)
        app_sensor_IMU_get(appMsg.data.sensorsReport.sensorsNotification.IMU_data);
        appMsg.data.sensorsReport.sensorsNotification.IMU_data_used = 0x01;
#endif

        appMsg.data.sensorsReport.sensorsNotification.number_of_active_LEDs = 0;
#if(REPORT_LED_STATUS)
        static uint8_t led_on_arr[APP_NUMBER_OF_LEDS] = {0};
        uint8_t num_leds_on                 = 0;
        num_leds_on = app_LED_get_selective(true, led_on_arr);
        appMsg.data.sensorsReport.sensorsNotification.number_of_active_LEDs = num_leds_on;
        memcpy(appMsg.data.sensorsReport.sensorsNotification.LEDs_active, led_on_arr, APP_NUMBER_OF_LEDS);
        TL_LOG_D("[APP] ### Reporting LEDs on %d", num_leds_on);
#endif

        appMsg.messageType = APP_MESSAGE_SENSORS_REPORT;
        appMsg.linkType = app_get_current_link_type();

        sidewalk_worker_take_job(SIDEWALK_WORKER_DO_SEND_MESSAGE, &appMsg);
        TL_LOG_D("[APP]Sensor report sent");
}

void app_send_LED_om_response(void) {
      int retVal;
        // Read led status
        static app_message_to_encode_t appMsg;
        static uint8_t led_on_arr[APP_NUMBER_OF_LEDS] = {0};
        uint8_t num_leds_on                 = 0;

        num_leds_on = app_LED_get_selective(true, led_on_arr);

        TL_LOG_D("[APP] ### Reporting LEDs on %d", num_leds_on);

        uint8_t x;

        for(x=0; x!=num_leds_on; x++) {
            TL_LOG_D("[APP] ### Reporting LEDs ID:%d", led_on_arr[x]);
        }

        appMsg.data.ledsResponse.LEDresponse.gps_time_in_seconds         = get_uptime();
        appMsg.data.ledsResponse.LEDresponse.resp_type                   = SID_DEMO_ACTION_TYPE_LED;
        appMsg.data.ledsResponse.LEDresponse.led_action_resp.action_resp = SID_DEMO_ACTION_LED_ON;
        appMsg.data.ledsResponse.LEDresponse.led_action_resp.num_leds    = num_leds_on;
        appMsg.data.ledsResponse.LEDresponse.led_action_resp.led_id_arr  = led_on_arr;

        appMsg.messageType = APP_MESSAGE_LEDS_RESPONSE;
        appMsg.linkType = app_get_current_link_type();

        sidewalk_worker_take_job(SIDEWALK_WORKER_DO_SEND_MESSAGE, &appMsg);
        TL_LOG_D("[APP] LED ON response sent");
}

void app_send_LED_off_response(void) {
        // Read led status
        int retVal;
        static app_message_to_encode_t appMsg;
        static uint8_t led_off_arr[APP_NUMBER_OF_LEDS] = {0};
        uint8_t num_leds_off                           = 0;

        num_leds_off = app_LED_get_selective(false, led_off_arr);

        appMsg.data.ledsResponse.LEDresponse.gps_time_in_seconds         = get_uptime();
        appMsg.data.ledsResponse.LEDresponse.resp_type                   = SID_DEMO_ACTION_TYPE_LED;
        appMsg.data.ledsResponse.LEDresponse.led_action_resp.action_resp = SID_DEMO_ACTION_LED_OFF;
        appMsg.data.ledsResponse.LEDresponse.led_action_resp.num_leds    = num_leds_off;
        appMsg.data.ledsResponse.LEDresponse.led_action_resp.led_id_arr  = led_off_arr;

        appMsg.messageType = APP_MESSAGE_LEDS_RESPONSE;
        appMsg.linkType = app_get_current_link_type();

        sidewalk_worker_take_job(SIDEWALK_WORKER_DO_SEND_MESSAGE, &appMsg);

        TL_LOG_D("[APP]Response LED OFF send");
}

void app_send_link_type_change_response(void) {
    static app_message_to_encode_t appMsg;

    appMsg.data.linkTypeResponse.linkTypeResponse.gps_time_in_seconds = get_uptime();
    appMsg.data.linkTypeResponse.linkTypeResponse.resp_type = SID_DEMO_ACTION_TYPE_LINK_TYPE_CHANGE;

    appMsg.messageType = APP_MESSAGE_LINK_TYPE_RESPONSE;
    appMsg.linkType = app_get_current_link_type();

    sidewalk_worker_take_job(SIDEWALK_WORKER_DO_SEND_MESSAGE, &appMsg);

    TL_LOG_D("[APP]Response Link Type Change send");
}

void app_LED_action_handler(struct sid_demo_msg *msg) {

    static struct sid_parse_state LEDActionParserState      = {0};
    uint8_t                        leds[APP_NUMBER_OF_LEDS] = {0};
    struct sid_demo_led_action_req ledAction                = {.led_id_arr = leds};


    sid_parse_state_init(&LEDActionParserState, msg->payload, msg->payload_size);
    sid_demo_app_action_req_deserialize(&LEDActionParserState, &ledAction);
    if (LEDActionParserState.ret_code != SID_ERROR_NONE) {
        TL_LOG_E("[APP] LED Action deserialize failed %d", LEDActionParserState.ret_code);
        return;
    }

    switch(ledAction.action_req) {
        case SID_DEMO_ACTION_LED_ON:
            app_set_LED_selective(true, ledAction.led_id_arr, ledAction.num_leds);
            app_send_LED_om_response();
            break;
        case SID_DEMO_ACTION_LED_OFF:
            app_set_LED_selective(false, ledAction.led_id_arr, ledAction.num_leds);
            app_send_LED_off_response();
            break;

        default:
            TL_LOG_E("[APP] LED Action not known: %d", ledAction.action_req);
    }

    // Prevent deep sleep while at least one LED is on
    if( app_LED_get_number_of_active() != 0) {
        noSleep = true;
    }
    else {
        noSleep = false;
    }
}

void app_message_handler(uint8_t *messageBody, uint8_t messageLen) {

    static struct sid_parse_state AppParserSstate = {0};
    struct sid_demo_msg_desc AppMesgDescriptor    = {0};

    static uint8_t        AppPayloadData[APP_PAYLOAD_BUFF_SIZE] = {0};
    struct sid_demo_msg   AppMsg = {.payload =AppPayloadData, .payload_size = 0 };


    sid_parse_state_init(&AppParserSstate, messageBody, messageLen);
    sid_demo_app_msg_deserialize(&AppParserSstate, &AppMesgDescriptor, &AppMsg);

    if(AppParserSstate.ret_code!=SID_ERROR_NONE) {
        TL_LOG_I("[APP] Incomming message deserialize failed!");
        return;
    }

    TL_LOG_D("[APP] Message Recieved: Class:%d OPcode:%d id:%d", AppMesgDescriptor.cmd_class, AppMesgDescriptor.opc, AppMesgDescriptor.cmd_id);

    switch (AppMesgDescriptor.opc) {
        case SID_DEMO_MSG_TYPE_WRITE:
            if(AppMesgDescriptor.cmd_id == SID_DEMO_APP_CLASS_CMD_ACTION) {
                  if(AppMsg.payload[0] == SID_DEMO_TAG_LINK_TYPE_SET_ACTION_REQ)
                    { // hijack for setting  link type
                        if(AppMsg.payload[1] < LINK_TYPE_MAX) {
                            extern void start_link_type_sellection(linkTypeSellected_t RATsellected, uint8_t RATselDelay);
                            app_send_link_type_change_response();
                            start_link_type_sellection(AppMsg.payload[1], 15);

                        }
                    }
                    else {
                         app_LED_action_handler(&AppMsg); // behave as usual
                    }

            }
            break;

        case SID_DEMO_MSG_TYPE_NOTIFY:
            if (AppMesgDescriptor.cmd_id == SID_DEMO_APP_ACTION_NOTIFICATION) {
                    TL_LOG_I("[APP] Demo message recieved");
                    if(AppMsg.payload[0] == 0xCE) {
                        uint8_t msgLen = AppMsg.payload[1];
                        if(msgLen > MAX_TEXT_MESSAGE_LEN) {
                            char mesgBuf[MAX_TEXT_MESSAGE_LEN];
                            memcpy(mesgBuf, &AppMsg.payload[2], msgLen);
                            LCD_showText(mesgBuf);
                        } else {
                            TL_LOG_E("[APP] Message is to long!");
                        }
                    } else {
                        TL_LOG_E("[APP] Tag not known:%d", AppMsg.payload[0]);
                    }
                }
            break;
        case SID_DEMO_MSG_TYPE_RESP:
                switch (AppMesgDescriptor.cmd_id) {
                    case SID_DEMO_APP_CLASS_CMD_CAP_DISCOVERY_ID:
                        if (AppMesgDescriptor.status_hdr_ind && AppMesgDescriptor.status_code == SID_ERROR_NONE) {
                            TL_LOG_D("[APP] Capability response received");
                            APPstate = APP_STATE_RUNNING;
                        } else {
                            TL_LOG_E("[APP] Capability Response recieved with fault: %d", AppMesgDescriptor.status_code);
                        }
                        break;
                    default:
                        TL_LOG_I("[APP] Response recieved. id:%d hdr_ind:%d sttaus:%d", AppMesgDescriptor.cmd_id,
                                                                                        AppMesgDescriptor.status_hdr_ind,
                                                                                        AppMesgDescriptor.status_code);
                        break;
                }
                break;
        default:
            TL_LOG_E("[APP] OPcode:%d not supported", AppMesgDescriptor.opc);
            break;

    } // switch (AppMesgDescriptor.opc)
}

_attribute_ble_data_retention_ bool LinkTypeSellectorWorking = false;
_attribute_ble_data_retention_ static uint32_t linkTypeSelTimeStamp;
_attribute_ble_data_retention_ static uint32_t linkTypeCnt = 0;
_attribute_ble_data_retention_ linkTypeSellected_t linkTypeSellected = LINK_TYPE_BLE;
_attribute_ble_data_retention_ static uint8_t firstStatusChange = 0x01;


static void link_type_sellector_animate_LEDs(linkTypeSellected_t linkTypeSellected, bool forceON) {
    static bool toggleState;
    toggleState = !toggleState;
    toggleState |=forceON; // if force ON set, do not toggle.

    switch(linkTypeSellected) {
        case LINK_TYPE_BLE:
            app_LED_single_off(LED1);
            toggleState ? app_LED_single_on(LED0) : app_LED_single_off(LED0);
            break;
        case LINK_TYPE_FSK:
            app_LED_single_off(LED0);
            toggleState ? app_LED_single_on(LED1) : app_LED_single_off(LED1);
            break;
        case LINK_TYPE_LORA:
            toggleState ? app_LED_single_on(LED0) : app_LED_single_off(LED0);
            toggleState ? app_LED_single_on(LED1) : app_LED_single_off(LED1);
    }
}

void start_link_type_sellection(linkTypeSellected_t initiallinkTypeSellected, uint8_t RATselDelay) {

    LinkTypeSellectorWorking = true;
    linkTypeSelTimeStamp = linkTypeCnt + RATselDelay;
    APP_WDT = 0;
    linkTypeSellected = initiallinkTypeSellected;

}

void app_sidewalk_link_change(linkTypeSellected_t linkTypeSellected) {

    static uint32_t defaultLinkMask;
    switch(linkTypeSellected) {
        case LINK_TYPE_BLE:
            defaultLinkMask = SID_LINK_TYPE_1; //BLE
            break;
        case LINK_TYPE_FSK:
            defaultLinkMask = SID_LINK_TYPE_2; //FSK
            break;
        case LINK_TYPE_LORA:
            defaultLinkMask = SID_LINK_TYPE_3; //LORA
            break;
        default:
            defaultLinkMask = SID_LINK_TYPE_1;

    }
    sidewalk_worker_take_job(SIDEWALK_WORKER_DO_CHANGE_LINK, &defaultLinkMask);
    APPstate = APP_STATE_NOT_CONNECTED;
}

extern uint32_t wakeup_cntr;
static void notify_timer_cb(TimerHandle_t th) {
    static int cnt = 0;
    cnt++;
    APP_WDT++;
    linkTypeCnt++;

      if(LinkTypeSellectorWorking) {

        if(linkTypeCnt >= linkTypeSelTimeStamp) {
            link_type_sellector_animate_LEDs(linkTypeSellected, true);
            LinkTypeSellectorWorking = false;
            app_sidewalk_link_change(linkTypeSellected);
            firstStatusChange = 0x01;
            return;
        }

        link_type_sellector_animate_LEDs(linkTypeSellected, false);
        if(link_type_switch_button_check()) { // button pressed
            linkTypeSellected++;
            if (linkTypeSellected >=  LINK_TYPE_MAX)  linkTypeSellected = LINK_TYPE_BLE;
            linkTypeSelTimeStamp = linkTypeCnt + RAT_SELLECTOR_DELAY;
            link_type_switch_button_consume();
        }
        return;
    }

    if(APP_WDT >= APP_WT_REBOOT_INTERVAL) {
        sys_reboot();
    }

    if (link_type_switch_button_check()) {
        noSleep = true;
        start_link_type_sellection(LINK_TYPE_BLE, RAT_SELLECTOR_DELAY);
        link_type_switch_button_consume();
     }


    if ((cnt == SENSOR_UPDATE_INTERVAL) || (triggerReportAfterConnection)) {
        switch(APPstate) {
            case APP_STATE_CONNECTED:
                app_send_capabilities();
                break;
            case APP_STATE_RUNNING:
                app_send_sensors_report();
                break;
        }
        TL_LOG_D("[APP] Sleeped: %d noSleep:%d", wakeup_cntr, noSleep);
        wakeup_cntr = 0;
        tlkapi_debug_handler();
        triggerReportAfterConnection = false;
        cnt = 0;
    }

    if (app_is_any_button_pressed() != 0) {
        app_send_button_report();
        app_consume_all_buttons();
    }
}

static void handle_on_event(bool in_isr, void *context) {
    sidewalk_worker_take_job(SIDEWALK_WORKER_DO_SID_LOOP, NULL);
}

static void handle_on_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context) {
    TL_LOG_D("[APP] Received message(type: %d, link_mode: %d, id: %u size %u)", (int)msg_desc->type, (int)msg_desc->link_mode, msg_desc->id, msg->size);

    if (msg_desc->type == SID_MSG_TYPE_RESPONSE && msg_desc->msg_desc_attr.rx_attr.is_msg_ack) {
        TL_LOG_D("[APP] Ack for msg id %d", msg_desc->id);
        APP_WDT = 0x00;
    } else {
        app_message_handler(msg->data, msg->size);
    }
}

static void handle_on_msg_sent(const struct sid_msg_desc *msg_desc, void *context) {
    TL_LOG_D("[APP] sent message(type: %d, id: %u)", (int)msg_desc->type, msg_desc->id);
}

static void handle_on_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context) {
    TL_LOG_D("[APP] Failed to send message(type: %d, id: %u) err:%d", (int)msg_desc->type, msg_desc->id, error);
}

static void handle_on_factory_reset(void *context) {
    ARG_UNUSED(context);

    TL_LOG_D("Factory reset notification received from sid api");
    if (sid_hal_reset(SID_HAL_RESET_NORMAL)) {
        TL_LOG_E("[APP] Reboot failed.");
    }
}

static const char *link_state_to_string(uint32_t link_status_mask, uint32_t link_type) {
    return (link_status_mask & link_type) ? "up" : "down";
}

static const char *enabled_to_string(bool enabled) {
    return enabled ? "enabled" : "disabled";
}

static void log_status_summary(const struct sid_status *status)
{

    const char *registered_s = (status->detail.registration_status == SID_STATUS_REGISTERED) ? "registered" : "not registered";
    const char *time_sync_s =  (status->detail.time_sync_status == SID_STATUS_TIME_SYNCED) ? "synced" : "not synced";

    TL_LOG_D(
        "Device status: %s, time_sync=%s",
        registered_s,
        time_sync_s);

    TL_LOG_D(
        "Link states: BLE=%s, FSK=%s, LoRa=%s",
        link_state_to_string(status->detail.link_status_mask, SID_LINK_TYPE_1),
        link_state_to_string(status->detail.link_status_mask, SID_LINK_TYPE_2),
        link_state_to_string(status->detail.link_status_mask, SID_LINK_TYPE_3));
}

static void log_supported_modes(const struct sid_status *status)
{
    static const char *link_names[SID_LINK_TYPE_MAX_IDX] = {
    [SID_LINK_TYPE_1_IDX] = "BLE",
    [SID_LINK_TYPE_2_IDX] = "FSK",
    [SID_LINK_TYPE_3_IDX] = "LoRa"};

    for (int i = 0; i < SID_LINK_TYPE_MAX_IDX; ++i) {
        enum sid_link_mode mode = (enum sid_link_mode)status->detail.supported_link_modes[i];

        if (mode == 0) {
            continue;
        }

        TL_LOG_D(
            "%s supported modes: cloud=%s, mobile=%s",
            link_names[i],
            enabled_to_string((mode & SID_LINK_MODE_CLOUD) != 0),
            enabled_to_string((mode & SID_LINK_MODE_MOBILE) != 0));
    }
}

static void handle_on_status_changed(const struct sid_status *status, void *context)
{

    int err = 0;

    TL_LOG_D("[APP] SID Status changed ->%d", status->state);

    app_set_current_link_type(status->detail.link_status_mask);

    if (SID_STATUS_TIME_SYNCED == status->detail.time_sync_status) {
        APPstate = APP_STATE_CONNECTED;
        if(firstStatusChange) {
            app_LED_all_OFF(); //LEDs are truned on at power up. They will go off on sucessful cloud connection.
            firstStatusChange = 0;
            APP_WDT = 0;
            triggerReportAfterConnection = true; // let's trigger capabilities report sending
            noSleep = false; // connected, allow sllep
            // extern void app_process_power_management(void);
            // app_process_power_management();
        }
    } else {
       TL_LOG_E("[APP] Failed to sync time!");
    }

    log_status_summary(status);
    log_supported_modes(status);
}

void app_recover_after_deep_sleep(void) {
    app_sensors_recover_after_deep_sleep();
    app_LED_recover_after_deep_sleep();
    app_buttons_init();
    app_buttons_pool();
}

void app_start(void) {
#if(TLKAPI_DEBUG_ENABLE)
    tlkapi_debug_init();
#endif
     if (app_buttons_init()) {
         TL_LOG_E("[APP] Cannot init buttons");
     }

    if (app_LED_init()) {
        TL_LOG_E("[APP] Cannot init leds");
    }

    if (app_sensors_init()) {
        TL_LOG_E("[APP] Cannot init sensors");
    }

    app_LED_all_ON(); // turn on all LEDs to indicate that the device is waiting for cloud connection.
                      // They will go off on successful connection.

    LCD_Init();
    char text[] = {"Ready."};
    LCD_showText(text);

    TL_LOG_E("[APP] READY.");

    static struct sid_event_callbacks event_callbacks = {
        .context           = NULL,
        .on_event          = handle_on_event,
        .on_msg_received   = handle_on_msg_received,
        .on_msg_sent       = handle_on_msg_sent,
        .on_send_error     = handle_on_send_error,
        .on_status_changed = handle_on_status_changed,
        .on_factory_reset  = handle_on_factory_reset,
    };

    sidewalk_woker_start(&event_callbacks);
    sidewalk_worker_take_job(SIDEWALK_WORKER_DO_PLATFORM_INIT, NULL);
    sidewalk_worker_take_job(SIDEWALK_WORKER_DO_BOOTSTRAP, NULL);

    TimerHandle_t myTimer = xTimerCreate("notifyTimer",
                                         pdMS_TO_TICKS(NOTIFY_TIMER_DURATION_MS),
                                         pdTRUE,
                                         NULL,
                                         notify_timer_cb
    );
    xTimerStart(myTimer, 0);
}
