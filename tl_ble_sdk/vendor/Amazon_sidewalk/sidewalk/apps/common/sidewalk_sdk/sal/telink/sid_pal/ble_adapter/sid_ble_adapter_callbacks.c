/********************************************************************************************************
 * @file    sid_ble_adapter_callbacks.c
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

/** @file sid_ble_adapter_callbacks.c
 *  @brief Common callbacks implementation for Sidewalk.
 */
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "sid_ble_adapter.h"
#include <sid_ble_adapter_callbacks.h>


#define CALLBACK_SET(__target_cb, __source_cb)                                                     \
    do {                                                                                       \
        if (NULL == __source_cb) {                                                         \
            return SID_ERROR_INVALID_ARGS;                                             \
        }                                                                                  \
        __target_cb = __source_cb;                                                         \
    } while (0)

static sid_pal_ble_data_callback_t data_cb;
static sid_pal_ble_notify_callback_t notify_changed_cb;
static sid_pal_ble_indication_callback_t notify_sent_cb;
static sid_pal_ble_connection_callback_t connection_cb;
static sid_pal_ble_mtu_callback_t mtu_changed_cb;
static sid_pal_ble_adv_start_callback_t adv_start_cb;

sid_error_t sid_ble_adapter_notification_cb_set(sid_pal_ble_indication_callback_t cb)
{
    CALLBACK_SET(notify_sent_cb, cb);
    return SID_ERROR_NONE;
}

void sid_ble_adapter_notification_sent(void)
{
    TL_LOG_D("BLE -> Sidewalk");
    if (notify_sent_cb) {
        notify_sent_cb(true);
    }
}

sid_error_t sid_ble_adapter_data_cb_set(sid_pal_ble_data_callback_t cb)
{
    CALLBACK_SET(data_cb, cb);
    return SID_ERROR_NONE;
}

void sid_ble_adapter_data_write(sid_ble_cfg_service_identifier_t id, uint8_t *data, uint16_t length)
{
    TL_LOG_D(" data :BLE -> Sidewalk");
    if (data_cb) {
        data_cb(id, data, length);
    }
}

sid_error_t sid_ble_adapter_notification_changed_cb_set(sid_pal_ble_notify_callback_t cb)
{
    CALLBACK_SET(notify_changed_cb, cb);
    return SID_ERROR_NONE;
}

void sid_ble_adapter_notification_changed(sid_ble_cfg_service_identifier_t id, bool state)
{
    TL_LOG_D("BLE -> Sidewalk");
    if (notify_changed_cb) {
        notify_changed_cb(id, state);
    }
}

sid_error_t sid_ble_adapter_conn_cb_set(sid_pal_ble_connection_callback_t cb)
{
    CALLBACK_SET(connection_cb, cb);
    return SID_ERROR_NONE;
}

void sid_ble_adapter_conn_connected(const uint8_t *ble_addr)
{
    TL_LOG_D("BLE -> Sidewalk");
    if (connection_cb) {
        connection_cb(true, (uint8_t *)ble_addr);
    }
}

void sid_ble_adapter_conn_disconnected(const uint8_t *ble_addr)
{
    TL_LOG_D("BLE -> Sidewalk");
    if (connection_cb) {
        connection_cb(false, (uint8_t *)ble_addr);
    }
}

sid_error_t sid_ble_adapter_mtu_cb_set(sid_pal_ble_mtu_callback_t cb)
{
    TL_LOG_D("BLE -> Sidewalk");
    CALLBACK_SET(mtu_changed_cb, cb);
    return SID_ERROR_NONE;
}

void sid_ble_adapter_mtu_changed(uint16_t mtu_size)
{
    TL_LOG_D("BLE -> Sidewalk");
    if (mtu_changed_cb) {
        mtu_changed_cb(mtu_size);
    }
}

sid_error_t sid_ble_adapter_adv_start_cb_set(sid_pal_ble_adv_start_callback_t cb)
{
    CALLBACK_SET(adv_start_cb, cb);
    return SID_ERROR_NONE;
}

void sid_ble_adapter_adv_started(void)
{
    TL_LOG_D("BLE -> Sidewalk");
    if (adv_start_cb) {
        adv_start_cb();
    }
}
