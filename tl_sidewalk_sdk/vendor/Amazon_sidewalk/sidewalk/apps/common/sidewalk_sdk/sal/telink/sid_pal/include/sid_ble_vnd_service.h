/********************************************************************************************************
 * @file    sid_ble_vnd_service.h
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
/** @file sid_ble_vnd_service.h
 *  @brief Vendor service.
 */

#ifndef SID_PAL_BLE_VND_SERVICE_H
#define SID_PAL_BLE_VND_SERVICE_H

#include <sid_ble_uuid.h>

/* VENDOR_SERVICE */
#define VND_SID_BT_UUID_SERVICE BT_UUID_DECLARE_16(VND_EXAMPLE_SERVICE_UUID_VAL)
#define VND_SID_BT_CHARACTERISTIC_WRITE                                                            \
    BT_UUID_DECLARE_128(VND_EXAMPLE_CHARACTERISTIC_UUID_VAL_WRITE)
#define VND_SID_BT_CHARACTERISTIC_NOTIFY                                                           \
    BT_UUID_DECLARE_128(VND_EXAMPLE_CHARACTERISTIC_UUID_VAL_NOTIFY)

uint16_t sid_ble_get_vnd_service_ccc_valueHandle(void);

#endif /* SID_PAL_BLE_VND_SERVICE_H */
