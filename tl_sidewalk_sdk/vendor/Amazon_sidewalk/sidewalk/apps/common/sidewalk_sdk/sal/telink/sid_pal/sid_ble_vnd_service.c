/********************************************************************************************************
 * @file    sid_ble_vnd_service.c
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
/** @file sid_ble_vnd_service.c
 *  @brief Bluetooth low energy vendor service implementation.
 */
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "sid_ble_adapter.h"
#include <sid_ble_vnd_service.h>
#include <sid_ble_adapter_callbacks.h>



int vnd_srv_on_write(uint16_t conn_handle, uint16_t handle, const void *buf, uint16_t len, uint16_t offset);


void vnd_srv_notif_changed(uint16_t conn_handle, uint16_t handle, uint16_t value)
{
    bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);
    ARG_UNUSED(handle);
    ARG_UNUSED(conn_handle);
    TL_LOG_D("Notification for VENDOR_SERVICE is %s.", notif_enabled ? "enabled" : "disabled");
    sid_ble_adapter_notification_changed(VENDOR_SERVICE, notif_enabled);
}

int vnd_srv_on_write(uint16_t conn_handle, uint16_t handle, const void *buf, uint16_t len, uint16_t offset)
{
    ARG_UNUSED(conn_handle);
    ARG_UNUSED(handle);
    ARG_UNUSED(offset);

    TL_LOG_D("Data received for VENDOR_SERVICE [len=%d].", len);

    sid_ble_adapter_data_write(VENDOR_SERVICE, (uint8_t *)buf, len);
    return len;
}


uint16_t sid_ble_get_vnd_service_ccc_valueHandle(void)
{

}
