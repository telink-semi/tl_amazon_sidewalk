/********************************************************************************************************
 * @file    sid_ble_advert.h
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
#ifndef SID_BLE_ADVERT_H
#define SID_BLE_ADVERT_H

#include <stdint.h>

/**
 * @brief Initialize Bluetooth Advertising.
 *
 * @return Zero on success or (negative) error code on failure.
 */
int sid_ble_advert_init(void);

/**
 * @brief Deinitialize Bluetooth Advertising.
 *
 * @return Zero on success or (negative) error code on failure.
 */
int sid_ble_advert_deinit(void);

/**
 * @brief Start Bluetooth advertising.
 *
 * @return Zero on success or (negative) error code on failure.
 */
int sid_ble_advert_start(void);

/**
 * @brief Notify sid_ble_advert that connection has been made
 * 
 */
void sid_ble_advert_notify_connection(void);

/**
 * @brief Stop Bluetooth advertising.
 *
 * @return Zero on success or (negative) error code on failure.
 */
int sid_ble_advert_stop(void);

/**
 * @brief Update advertising data.
 *
 * @note update the value of manufacturing section in ble advertising.
 * Data may be trimmed to meet bluetooth advertising size requirements.
 * Too long manufacuring data may affect Device Name.
 *
 * @param data buffor of data to be updated.
 * @param data_len length of data to be updated in bytes.
 *
 * @return Zero on success or (negative) error code on failure.
 */
int sid_ble_advert_update(uint8_t *data, uint8_t data_len);

/**
 * @brief Get Bluetooth advertising State.
 *
 * @return 0 of disable,1 is fast , 2 is slow.
 */
int sid_ble_get_adv_state(void);

int sid_ble_set_adv_param(uint32_t ble_adv_fast_interval, uint32_t ble_adv_fast_timeout, \
      uint32_t  ble_adv_slow_interval, uint32_t  ble_adv_slow_timeout);

#endif /* SID_BLE_ADVERT_H */
