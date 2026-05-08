/********************************************************************************************************
 * @file    sid_ble_service.c
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
#include <sid_ble_service.h>
#include <sid_ble_adapter_callbacks.h>


 void notification_sent(void)
{
    TL_LOG_D("Notification sent.");
    sid_ble_adapter_notification_sent();
}

int sid_ble_send_data(sid_ble_srv_params_t *params, uint8_t *data, uint16_t length)
{
    ble_sts_t rt = BLE_SUCCESS;

    if (!params) {
        return -ENOENT;
    }

    if(params->isNotify)
    {
        rt = blc_gatt_pushHandleValueNotify (params->connHandle, params->valueHandle, data, length);
        if(GATT_ERR_DATA_PENDING_DUE_TO_SERVICE_DISCOVERY_BUSY == rt )
        {
              blc_gap_setSingleServerDataPendingTime_upon_ClientCmd(params->connHandle,0);
              rt = blc_gatt_pushHandleValueNotify (params->connHandle, params->valueHandle, data, length);
        }
        notification_sent();
    }
    else
    {
        rt = blc_gatt_pushHandleValueIndicate (params->connHandle, params->valueHandle, data, length);
        if(GATT_ERR_DATA_PENDING_DUE_TO_SERVICE_DISCOVERY_BUSY == rt )
        {
              blc_gap_setSingleServerDataPendingTime_upon_ClientCmd(params->connHandle,0);
              rt = blc_gatt_pushHandleValueIndicate (params->connHandle, params->valueHandle, data, length);
        }
    }

    if (rt) {
        TL_LOG_E("Send err:%d.", rt);
    }
    return rt;
}
