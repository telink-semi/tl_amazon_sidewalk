/********************************************************************************************************
 * @file    sid_ble_ama_service.c
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
#include "stack/ble/ble.h"
#include "sid_ble_adapter.h"
#include <sid_ble_ama_service.h>
#include <sid_ble_adapter_callbacks.h>
#include "app_att.h"


void ama_srv_notif_changed(uint16_t conn_handle, uint16_t handle,  uint16_t  value);
int ama_srv_on_write(uint16_t conn_handle, uint16_t handle, const void *buf, uint16_t len, uint16_t offset);


void ama_srv_notif_changed(uint16_t conn_handle, uint16_t handle,  uint16_t  value)
{
    bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);
    TL_LOG_D("Notification %d %d %s", handle,AMA_SIDNTF_DP_H,notif_enabled ? "enabled" : "disabled");
    sid_ble_adapter_notification_changed(AMA_SERVICE, notif_enabled);
}


int ama_srv_on_write(uint16_t conn_handle, uint16_t handle, const void *buf, uint16_t len, uint16_t offset)
// ssize_t ama_srv_on_write(struct bt_conn *conn, const struct bt_gatt_attr *attr,
//                const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    ARG_UNUSED(offset);
    TL_LOG_D("Data received for AMA_SERVICE [len=%d].", len);
    sid_ble_adapter_data_write(AMA_SERVICE, (uint8_t *)buf, len);
    return len;
}



uint16_t sid_ble_get_ama_service_ccc_valueHandle(void)
{
    return AMA_SIDNTF_DP_H;
}
