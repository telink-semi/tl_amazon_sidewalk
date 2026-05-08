/********************************************************************************************************
 * @file    tlv_ram_storage_impl.c
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

int tlv_storage_ram_write(void *ctx, uint32_t offset, uint8_t *data, uint32_t data_size)
{
    uint8_t *ram_buffer = (uint8_t *)ctx;
    memcpy(ram_buffer + offset, data, data_size);
    return 0;
}
int tlv_storage_ram_read(void *ctx, uint32_t offset, uint8_t *data, uint32_t data_size)
{
    uint8_t *ram_buffer = (uint8_t *)ctx;
    memcpy(data, ram_buffer + offset, data_size);
    return 0;
}

int tlv_storage_ram_erase(void *ctx, uint32_t offset, uint32_t size)
{
    uint8_t *ram_buffer = (uint8_t *)ctx;
    memset(ram_buffer + offset, 0x00, size);
    return 0;
}
