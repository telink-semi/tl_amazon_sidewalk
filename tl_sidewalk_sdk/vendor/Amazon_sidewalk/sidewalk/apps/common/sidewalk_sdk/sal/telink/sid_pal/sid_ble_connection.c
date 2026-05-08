/********************************************************************************************************
 * @file    sid_ble_connection.c
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
#include <sid_ble_connection.h>
#include <sid_ble_adapter_callbacks.h>
#include <sid_ble_advert.h>

#include <errno.h>
#include <bt_app_callbacks.h>
#if (FREERTOS_ENABLE)
    #include "tlk_riscv.h"
    #include <FreeRTOS.h>
    #include <task.h>
    #include <timers.h>
    #include <queue.h>
    #include <semphr.h>
    #include <event_groups.h>
    #include "app_freertos.h"
#endif

#if (FREERTOS_ENABLE)
_attribute_ble_data_retention_ static StaticSemaphore_t xMutexBuffer;
_attribute_ble_data_retention_ static SemaphoreHandle_t xMutex;
#endif


#define INVALID_HANDLE 0xFFFF
 void ble_connect_cb(uint16_t connHandle, uint8_t* addr, uint8_t err);
 void ble_disconnect_cb(uint16_t connHandle, uint8_t reason);
 void ble_mtu_cb(uint16_t connHandle, uint16_t tx_mtu, uint16_t rx_mtu);

static sid_ble_conn_params_t conn_params;
static sid_ble_conn_params_t *p_conn_params_out;


/**
 * @brief Check if the connection came from right adv, and if it is valid.
 * 
 * @param conn connection to check
 * @return true if the connection should be handled by Sidewalk
 * @return false connection is not for Sidewlak
 */
static bool is_connection_valid(uint16_t connHandle)
{
//    struct bt_conn_info conn_info = {};
//
//    if (!conn || bt_conn_get_info(conn, &conn_info) || conn_info.id != BT_ID_SIDEWALK) {
//        return false;
//    }
//todo
    return true;
}

uint16_t sid_get_connhandle(void)
{
    return conn_params.connHandle;
}

/**
 * @brief The function is called when a new connection is established.
 *
 * @param conn new connection object.
 * @param err HCI error, zero for success, non-zero otherwise.
 */
void ble_connect_cb(uint16_t connHandle, uint8_t* addr, uint8_t err)
{

    if (!is_connection_valid(connHandle)) {
        return;
    }
    if (err) {
        TL_LOG_E("Connection failed (err %u)\n", err);
        return;
    }
    sid_ble_advert_notify_connection();
    if (addr) {
        memcpy(conn_params.addr, addr, 6);
    } else {
        TL_LOG_E("Connection bt address not found.");
        memset(conn_params.addr, 0x00, 6);
    }

    #if (FREERTOS_ENABLE)
    if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
    {
    #endif
    conn_params.connHandle = connHandle;

    sid_ble_adapter_conn_connected((const uint8_t *)conn_params.addr);
    #if (FREERTOS_ENABLE)
    }
    xSemaphoreGive(xMutex);
    #endif

    TL_LOG_I("BT Connected");
}

/**
 * @brief The function is called when a connection has been disconnected.
 *
 * @param conn connection object.
 * @param err HCI disconnection reason.
 */
void ble_disconnect_cb(uint16_t connHandle, uint8_t reason)
{
    if (!is_connection_valid(connHandle) || conn_params.connHandle != connHandle) {
        return;
    }

    sid_ble_adapter_conn_disconnected((const uint8_t *)conn_params.addr);

    #if (FREERTOS_ENABLE)
    if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
    {
    #endif
    conn_params.connHandle = INVALID_HANDLE;
    #if (FREERTOS_ENABLE)
    }
    xSemaphoreGive(xMutex);
    #endif

    TL_LOG_I("BT Disconnected Reason: 0x%x ", reason);
}

void ble_mtu_cb(uint16_t connHandle, uint16_t tx_mtu, uint16_t rx_mtu)
{
    ARG_UNUSED(rx_mtu);

    if (conn_params.connHandle == connHandle) {
        sid_ble_adapter_mtu_changed(MIN(tx_mtu, rx_mtu));
    }
}

const sid_ble_conn_params_t *sid_ble_conn_params_get(void)
{
    return (const sid_ble_conn_params_t *)p_conn_params_out;
}

void sid_ble_conn_init(void)
{
    p_conn_params_out = &conn_params;
    static bool bt_conn_registered;

    if (!bt_conn_registered) {

        #if (FREERTOS_ENABLE)
        xMutex = xSemaphoreCreateMutexStatic(&xMutexBuffer);
        #endif
        bt_conn_registered = true;
        p_conn_params_out->connHandle = 0xFFFF;

    }
}

int sid_ble_conn_disconnect(void)
{
    int rt = 0;
    if (INVALID_HANDLE == conn_params.connHandle ) {
        return -ENOENT;
    }

    #if (FREERTOS_ENABLE)
    if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
    {
    #endif

        rt = blc_ll_disconnect(conn_params.connHandle, HCI_ERR_CONN_TERM_BY_LOCAL_HOST);

    #if (FREERTOS_ENABLE)
    }
    xSemaphoreGive(xMutex);
    #endif

    return rt;
}

void sid_ble_conn_deinit(void)
{
    p_conn_params_out = NULL;
}
