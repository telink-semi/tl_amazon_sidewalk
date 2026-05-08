/********************************************************************************************************
 * @file    bt_app_callbacks.c
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
#include <bt_app_callbacks.h>
#include <stdbool.h>
#include <sid_ble_uuid.h>
#include "sid_ble_adapter.h"

_attribute_ble_data_retention_ static uint32_t bt_enable_count = 0;

int sid_ble_bt_enable(void)
{
    if (bt_enable_count == 0) {
        int ret = blc_bt_enable();
        if (ret == 0) {
//            if (cb) {
//                cb(0);
//            }
            bt_enable_count++;
        }
        return ret;
    }

    bt_enable_count++;
//    if (cb) {
//        cb(0);
//    }
    return 0;
}

int sid_ble_bt_disable()
{
    if (bt_enable_count <= 0) {
        bt_enable_count = 0;
        return -EALREADY;
    }
    if (bt_enable_count == 1) {
        bt_enable_count = 0;
        return blc_bt_disable();
    } else {
        bt_enable_count--;
        return 0;
    }
}

bool sid_ble_is_enable(void)
{
    if(bt_enable_count > 0)
        return true;
    return false;
}
