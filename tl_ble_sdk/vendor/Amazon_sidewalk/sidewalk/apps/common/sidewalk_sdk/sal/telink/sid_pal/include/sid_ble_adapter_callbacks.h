/********************************************************************************************************
 * @file    sid_ble_adapter_callbacks.h
 *
 * @brief   This is the header file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2022
 *
 * @par     Copyright (c) 2022, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
/** @file sid_ble_adapter_callbacks.h
 *  @brief Common callbacks API for Sidewalk.
 */

#ifndef SID_PAL_BLE_ADAPTER_CALLBACKS_H
#define SID_PAL_BLE_ADAPTER_CALLBACKS_H

#include <sid_error.h>
#include <sid_pal_ble_adapter_ifc.h>

/**
 * @brief Set a callback for notification sent.
 *
 * @param cb  a callback to function which should be call.
 * @return SID_ERROR_NONE when success, error code otherwise.
 */
sid_error_t sid_ble_adapter_notification_cb_set(sid_pal_ble_indication_callback_t cb);

/**
 * @brief Execution notification callback.
 *
 */
void sid_ble_adapter_notification_sent(void);

/**
 * @brief Set a callback for a data handling.
 *
 * @param cb  a callback to function which should be call.
 * @return SID_ERROR_NONE when success, error code otherwise.
 */
sid_error_t sid_ble_adapter_data_cb_set(sid_pal_ble_data_callback_t cb);

/**
 * @brief Execute callback after data receiving.
 *
 * @param id service identifier.
 * @param data buffer with data.
 * @param length data buffer length.
 */
void sid_ble_adapter_data_write(sid_ble_cfg_service_identifier_t id, uint8_t *data,
                uint16_t length);

/**
 * @brief Set a callback for notification subscription change.
 *
 * @param cb  a callback to function which should be call.
 * @return SID_ERROR_NONE when success, error code otherwise.
 */
sid_error_t sid_ble_adapter_notification_changed_cb_set(sid_pal_ble_notify_callback_t cb);

/**
 * @brief Execute callback when notification has been changed.
 *
 * @param id service identifier.
 * @param state notification state.
 */
void sid_ble_adapter_notification_changed(sid_ble_cfg_service_identifier_t id, bool state);

/**
 * @brief Set a callback for connection change.
 *
 * @param cb a callback to function which should be call.
 * @return SID_ERROR_NONE when success, error code otherwise.
 */
sid_error_t sid_ble_adapter_conn_cb_set(sid_pal_ble_connection_callback_t cb);

/**
 * @brief Execute callback after BLE device connected.
 *
 * @param ble_addr BLE address.
 */
void sid_ble_adapter_conn_connected(const uint8_t *ble_addr);

/**
 * @brief Execute callback after BLE device disconnected.
 *
 * @param ble_addr BLE address.
 */
void sid_ble_adapter_conn_disconnected(const uint8_t *ble_addr);

/**
 * @brief Set a callback for mtu change.
 *
 * @param cb a callback to function which should be call.
 * @return SID_ERROR_NONE when success, error code otherwise.
 */
sid_error_t sid_ble_adapter_mtu_cb_set(sid_pal_ble_mtu_callback_t cb);

/**
 * @brief Execute callback after mtu changed.
 *
 * @param mtu_size new mtu size.
 */
void sid_ble_adapter_mtu_changed(uint16_t mtu_size);

/**
 * @brief Set a callback for advertising start.
 *
 * @param cb a callback to function which should be call.
 * @return SID_ERROR_NONE when success, error code otherwise.
 */
sid_error_t sid_ble_adapter_adv_start_cb_set(sid_pal_ble_adv_start_callback_t cb);

/**
 * @brief Execute callback after advertising started.
 *
 */
void sid_ble_adapter_adv_started(void);

#endif /* SID_PAL_BLE_ADAPTER_CALLBACKS_H */
