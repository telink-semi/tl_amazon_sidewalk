/********************************************************************************************************
 * @file    sid_ble_service.h
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
/** @file sid_ble_service.h
 *  @brief Bluetooth low energy service API.
 */

#ifndef SID_PAL_BLE_SERVICE_H
#define SID_PAL_BLE_SERVICE_H


typedef struct {
    uint16_t connHandle;
    uint16_t valueHandle;
    bool isNotify;
} sid_ble_srv_params_t;

/**
 * @brief Send data over BLE.
 *
 * @param params service parameters.
 * @param data buffer with data.
 * @param length data buffer length.
 * @return 0 in case of success, negative value otherwise.
 */
int sid_ble_send_data(sid_ble_srv_params_t *params, uint8_t *data, uint16_t length);

#endif /* SID_PAL_BLE_SERVICE_H */
