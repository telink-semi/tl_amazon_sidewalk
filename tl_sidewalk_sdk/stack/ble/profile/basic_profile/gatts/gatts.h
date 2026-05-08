/********************************************************************************************************
 * @file    gatts.h
 *
 * @brief   This is the header file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    04,2024
 *
 * @par     Copyright (c) 2024, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#pragma once

#include "gatts_client_buf.h"
#include "gatts_server_buf.h"

/******************************* GATT Common Start **********************************************************************/

typedef struct __attribute__((packed)) {
    u16 startHandle;
    u16 endHandle;
} blc_gatt_service_changed_t;

typedef struct __attribute__((packed)) {
    u8 hash[16];
} blc_gatt_database_hash_t;

typedef enum {
    BLC_GATT_CLIENT_FEATURE_OCTET_0_ROBUST_CACHING = 0x01,
    BLC_GATT_CLIENT_FEATURE_OCTET_0_EATT_BEARER = 0x02,
    BLC_GATT_CLIENT_FEATURE_OCTET_0_MULTIPLE_HANDLE_VALUE_NOTIFICATIONS = 0x04,
} blc_gatt_client_feature_t;

/******************************* GATT Common End **********************************************************************/

/******************************* GATT Client Start **********************************************************************/
//GATT Client Event ID
typedef enum {
    BASIC_EVT_GATTSC_START = BASIC_EVT_TYPE_GATT_SERVICE_CLIENT,
    BASIC_EVT_GATTSC_RECV_SERVICE_CHANGED_INDICATION,
} basic_gattsc_evt_enum;

typedef struct { //Event ID: BASIC_EVT_GATTC_RECV_SERVICE_CHANGED_INDICATION
    blc_gatt_service_changed_t svc_changed;
} blc_gattsc_serviceChangedIndication_t;

/**
 * @brief       This function serves to register GATT Client function
 * @param[in]   currently not used, input NULL
 * @return      none.
 */
void blc_basic_registerGATTSControlClient(const blc_gattsc_regParam_t *param);

////GATTS Client Read Characteristic Value Operation API
/**
 * @brief      Read the supported features of the GATT server client.
 * @param[in]  connHandle - The connection handle for the GATT server client.
 * @param[in]  readCb - Callback function to handle the result of the read operation.
 * @return     ble_sts_t - Status of the read operation, 0x00: succeed, other: failed.
 */
ble_sts_t blc_gattsc_readClientSupportedFeatures(u16 connHandle, prf_read_cb_t readCb);

/**
 * @brief      Read the database hash of the GATT server client.
 * @param[in]  connHandle - The connection handle for the GATT server client.
 * @param[in]  readCb - Callback function to handle the result of the read operation.
 * @return     ble_sts_t - Status of the read operation, 0x00: succeed, other: failed.
 */
ble_sts_t blc_gattsc_readDatabaseHash(u16 connHandle, prf_read_cb_t readCb);

////GATTS Client Write Characteristic Value Operation API
/**
 * @brief      Write the supported features to the GATT server client.
 * @param[in]  connHandle - The connection handle for the GATT server client.
 * @param[in]  features - Pointer to the array containing the features to be written.
 * @param[in]  featuresLength - The length of the features array.
 * @param[in]  writeCb - Callback function to handle the result of the write operation.
 * @return     ble_sts_t - Status of the write operation, 0x00: succeed, other: failed.
 */
ble_sts_t blc_gattsc_writeClientSupportedFeatures(u16 connHandle, u8 *features, u16 featuresLength, prf_write_cb_t writeCb);

////GATTS Client Get Characteristic Value Operation API
/**
 * @brief      Get the supported features of the GATT server client.
 * @param[in]  connHandle - The connection handle for the GATT server client.
 * @param[out] features - Pointer to the buffer where the supported features will be stored.
 * @param[out] featuresLength - Pointer to the variable where the length of the features will be stored.
 * @return     ble_sts_t - Status of the operation, 0x00: succeed, other: failed.
 */
ble_sts_t blc_gattsc_getClientSupportedFeatures(u16 connHandle, u8* features, u16 *featuresLength);

/**
 * @brief      Get the database hash of the GATT server client.
 * @param[in]  connHandle - The connection handle for the GATT server client.
 * @param[out] databaseHash - Pointer to the structure where the database hash will be stored.
 * @return     ble_sts_t - Status of the operation, 0x00: succeed, other: failed.
 */
ble_sts_t blc_gattsc_getDatabaseHash(u16 connHandle, blc_gatt_database_hash_t *databaseHash);

/******************************* GATT Client End **********************************************************************/
