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
#include <app.h>
#include <sidewalk.h>
#include <app_ble_config.h>
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
#include <app_subGHz_config.h>
#endif
#include <sid_hal_reset_ifc.h>
#include <stdbool.h>


#include <bt_app_callbacks.h>



#include "app_ui.h"

#define KEY1  0xf1
#define KEY2  0xf1


#define PARAM_UNUSED (0U)

static uint32_t persistent_link_mask = SID_LINK_TYPE_1;  //ble

static void on_sidewalk_event(bool in_isr, void *context)
{
    int err = sidewalk_event_send(sidewalk_event_process, NULL, NULL);
    if (err) {
        TL_LOG_E("Send event err %d", err);
    };
}

static void free_sid_echo_event_ctx(void *ctx)
{
    sidewalk_msg_t *echo = (sidewalk_msg_t *)ctx;
    if (echo == NULL) {
        return;
    }
    if (echo->msg.data) {
        sid_free(echo->msg.data);
    }
    sid_free(echo);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg,
                     void *context)
{
    tlkapi_send_string_data(1,"Message received success",(uint8_t *)msg->data, msg->size);

#ifdef CONFIG_SID_END_DEVICE_ECHO_MSGS
    if (msg_desc->type == SID_MSG_TYPE_GET || msg_desc->type == SID_MSG_TYPE_SET) {
        TL_LOG_I("Send echo message");
        sidewalk_msg_t *echo = sid_malloc(sizeof(sidewalk_msg_t));
        if (!echo) {
            TL_LOG_E("Failed to allocate event context for echo message");
            return;
        }
        memset(echo, 0x0, sizeof(*echo));
        echo->msg.size = msg->size;
        echo->msg.data = sid_malloc(echo->msg.size);
        if (!echo->msg.data) {
            TL_LOG_E("Failed to allocate memory for message echo data");
            sid_free(echo);
            return;
        }
        memcpy(echo->msg.data, msg->data, echo->msg.size);

        echo->desc.type = (msg_desc->type == SID_MSG_TYPE_GET) ? SID_MSG_TYPE_RESPONSE :
                                     SID_MSG_TYPE_NOTIFY;
        echo->desc.id =
            (msg_desc->type == SID_MSG_TYPE_GET) ? msg_desc->id : msg_desc->id + 1;
        echo->desc.link_type = SID_LINK_TYPE_ANY;
        echo->desc.link_mode = SID_LINK_MODE_CLOUD;

        int err =
            sidewalk_event_send(sidewalk_event_send_msg, echo, free_sid_echo_event_ctx);
        if (err) {
            free_sid_echo_event_ctx(echo);
            TL_LOG_E("Send event err %d", err);
        } else {
        }
    };
#endif /* CONFIG_SID_END_DEVICE_ECHO_MSGS */
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
{
    TL_LOG_I("Message send success");

}

static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc,
                   void *context)
{
    TL_LOG_E("Message send err %d ", (int)error);

}

static void on_sidewalk_factory_reset(void *context)
{
    ARG_UNUSED(context);
#ifndef CONFIG_SID_END_DEVICE_CLI
    TL_LOG_I("Factory reset notification received from sid api");
    if (sid_hal_reset(SID_HAL_RESET_NORMAL)) {
        TL_LOG_W("Cannot reboot");
    }
#else
    TL_LOG_I("sid_set_factory_reset success");
#endif
}

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
{
    int err = 0;
    uint32_t new_link_mask = status->detail.link_status_mask;
    struct sid_status *new_status = sid_malloc(sizeof(struct sid_status));
    if (!new_status) {
        TL_LOG_E("Failed to allocate memory for new status value");
    } else {
        memcpy(new_status, status, sizeof(struct sid_status));
    }
    err = sidewalk_event_send(sidewalk_event_new_status, new_status, sid_free);

    TL_LOG_I("Device %sregistered, Time Sync %s, Link status: {BLE: %s, FSK: %s, LoRa: %s}",
        (SID_STATUS_REGISTERED == status->detail.registration_status) ? "Is " : "Un",
        (SID_STATUS_TIME_SYNCED == status->detail.time_sync_status) ? "Success" : "Fail",
        (new_link_mask & SID_LINK_TYPE_1) ? "Up" : "Down",
        (new_link_mask & SID_LINK_TYPE_2) ? "Up" : "Down",
        (new_link_mask & SID_LINK_TYPE_3) ? "Up" : "Down");

    for (int i = 0; i < SID_LINK_TYPE_MAX_IDX; i++) {
        enum sid_link_mode mode =
            (enum sid_link_mode)status->detail.supported_link_modes[i];

        if (mode) {
            TL_LOG_I("Link mode on %s = {Cloud: %s, Mobile: %s}",
                (SID_LINK_TYPE_1_IDX == i) ? "BLE" :
                (SID_LINK_TYPE_2_IDX == i) ? "FSK" :
                (SID_LINK_TYPE_3_IDX == i) ? "LoRa" :
                                 "unknow",
                (mode & SID_LINK_MODE_CLOUD) ? "True" : "False",
                (mode & SID_LINK_MODE_MOBILE) ? "True" : "False");
        }
    }
}
static void free_sid_hello_event_ctx(void *ctx)
{
    sidewalk_msg_t *hello = (sidewalk_msg_t *)ctx;
    if (hello == NULL) {
        return;
    }
    if (hello->msg.data) {
        sid_free(hello->msg.data);
    }
    sid_free(hello);
}

static void app_btn_send_msg(uint32_t unused)
{
    ARG_UNUSED(unused);

    TL_LOG_I("Send hello message");
    const char payload[] = "hello";
    sidewalk_msg_t *hello = sid_malloc(sizeof(sidewalk_msg_t));
    if (!hello) {
        TL_LOG_E("Failed to alloc memory for message context");
        return;
    }
    memset(hello, 0x0, sizeof(*hello));

    hello->msg.size = sizeof(payload);
    hello->msg.data = sid_malloc(hello->msg.size);
    if (!hello->msg.data) {
        sid_free(hello);
        TL_LOG_E("Failed to allocate memory for message data");
        return;
    }
    memcpy(hello->msg.data, payload, hello->msg.size);

    hello->desc.type = SID_MSG_TYPE_NOTIFY;
    hello->desc.link_type = SID_LINK_TYPE_ANY;
    hello->desc.link_mode = SID_LINK_MODE_CLOUD;

    int err = sidewalk_event_send(sidewalk_event_send_msg, hello, free_sid_hello_event_ctx);
    if (err) {
        free_sid_hello_event_ctx(hello);
        TL_LOG_E("Send event err %d", err);
    } else {
    }
}

static void app_event_exit_dfu_mode(sidewalk_ctx_t *sid, void *ctx)
{
    int err = -ENOTSUP;
    if (err) {
        TL_LOG_E("dfu stop err %d", err);
    }
}

static void app_event_enter_dfu_mode(sidewalk_ctx_t *sid, void *ctx)
{
    ARG_UNUSED(sid);
    ARG_UNUSED(ctx);
}

static void app_btn_dfu_state(uint32_t unused)
{
    ARG_UNUSED(unused);
    static bool go_to_dfu_state = true;
    if (go_to_dfu_state) {
        sidewalk_event_send(app_event_enter_dfu_mode, NULL, NULL);
    } else {
        sidewalk_event_send(app_event_exit_dfu_mode, NULL, NULL);
    }

    go_to_dfu_state = !go_to_dfu_state;
}

static void app_btn_connect(uint32_t unused)
{
    ARG_UNUSED(unused);
    (void)sidewalk_event_send(sidewalk_event_connect, NULL, NULL);
}

static void app_btn_factory_reset(uint32_t unused)
{
    ARG_UNUSED(unused);
    (void)sidewalk_event_send(sidewalk_event_factory_reset, NULL, NULL);
}

static void app_btn_link_switch(uint32_t unused)
{
    ARG_UNUSED(unused);
    (void)sidewalk_event_send(sidewalk_event_link_switch, NULL, NULL);
}


void Portble_btn_press(u8 key)
{
    if(KEY1 == key)
    app_btn_send_msg(0);
    else
        app_btn_connect(0);
}
void Portble_btn_d_press(u8 key)
{
    app_btn_link_switch(0);
}
void Portble_btn_l_press(u8 key)
{
    if(KEY1 == key)
    app_btn_dfu_state(0);
    else
    app_btn_factory_reset(0);

}

#define MAX_TIME_SYNC_INTERVALS 10
static uint16_t default_sync_intervals_h[MAX_TIME_SYNC_INTERVALS] = { 2, 4, 8,
                                      12 }; // default GCS intervals
static struct sid_time_sync_config default_time_sync_config = {
    .adaptive_sync_intervals_h = default_sync_intervals_h,
    .num_intervals = sizeof(default_sync_intervals_h) / sizeof(default_sync_intervals_h[0]),
};

void app_start(void)
{
    static sidewalk_ctx_t sid_ctx = { 0 };
    sidewalk_start(&sid_ctx);

}
