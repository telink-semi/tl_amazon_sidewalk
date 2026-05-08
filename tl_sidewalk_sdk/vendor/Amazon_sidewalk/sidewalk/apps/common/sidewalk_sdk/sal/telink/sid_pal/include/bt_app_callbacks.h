/********************************************************************************************************
 * @file    bt_app_callbacks.h
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
#ifndef SID_PAL_APP_CALLBACKS_H
#define SID_PAL_APP_CALLBACKS_H

#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Wrapper for @bt_enable, with reference tracking.
 * Real @bt_enable is called only of first call to app_bt_enable
 * 
 * @param cb callback passed to @bt_enable
 * @return int result from @bt_enable call or 0 if called multiple times
 */
int sid_ble_bt_enable(void);

/**
 * @brief Wrapper for @bt_disable.
 * This function removes internal reference.
 * If the internal reference counter shows 0, real @bt_disable is called
 * 
 * @return int result from @bt_disable or 0 if sid_ble_bt_enable has been called more than sid_ble_bt_disable
 */
int sid_ble_bt_disable(void);




#ifdef __cplusplus
}
#endif
#endif
