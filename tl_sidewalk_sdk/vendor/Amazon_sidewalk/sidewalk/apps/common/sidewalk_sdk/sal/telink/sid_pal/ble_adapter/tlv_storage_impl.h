/********************************************************************************************************
 * @file    tlv_storage_impl.h
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
#ifndef TLV_STORAGE_IMPL_H
#define TLV_STORAGE_IMPL_H

#include "tl_common.h"


#if CONFIG_SIDEWALK_TLV_RAM
int tlv_storage_ram_write(void *ctx, uint32_t offset, uint8_t *data, uint32_t data_size);
int tlv_storage_ram_read(void *ctx, uint32_t offset, uint8_t *data, uint32_t data_size);
int tlv_storage_ram_erase(void *ctx, uint32_t offset, uint32_t size);
#endif

#if CONFIG_SIDEWALK_TLV_FLASH
int tlv_storage_flash_write(void *ctx, uint32_t offset, uint8_t *data, uint32_t data_size);
int tlv_storage_flash_read(void *ctx, uint32_t offset, uint8_t *data, uint32_t data_size);
int tlv_storage_flash_erase(void *ctx, uint32_t offset, uint32_t size);
#endif

#endif
