/********************************************************************************************************
 * @file    sid_ble_connection.h
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
#ifndef SID_BLE_CONNECTION_H
#define SID_BLE_CONNECTION_H


/**
 * @brief Struct with bluetooth connection paramters.
 */
typedef struct {
    uint16_t connHandle;
    uint8_t addr[6];
} sid_ble_conn_params_t;

/**
 * @brief Initialize ble connection module.
 */
void sid_ble_conn_init(void);

/**
 * @brief Disconnect from a remote device or cancel pending connection.
 *
 * @return Zero on success or (negative) error code on failure.
 */
int sid_ble_conn_disconnect(void);

/**
 * @brief Deinitialize ble connection module.
 */
void sid_ble_conn_deinit(void);

/**
 * @brief The function returns current connection paramters.
 *
 * @return connection paramters as defined in @ref sid_ble_conn_params_t.
 */
const sid_ble_conn_params_t *sid_ble_conn_params_get(void);

#endif /* SID_BLE_CONNECTION_H */
