/********************************************************************************************************
 * @file    tlv.h
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
#ifndef TLV_H
#define TLV_H

#include <stdint.h>
#include <stdbool.h>

#define TLV_HDR_LEN           4u
#define TLV_ALIGN             4u
#define TLV_PAD_BYTE          0xFFu
#define TLV_EOF_TAG           0xFFFFu

typedef struct tlv_ctx {
    int (*write)(void *ctx, uint32_t offset, uint8_t *data, uint32_t data_size);
    int (*read)(void *ctx, uint32_t offset, uint8_t *data, uint32_t data_cap);
    int (*erase)(void *ctx, uint32_t offset, uint32_t size);
    void *ctx;
} tlv_io_t;


typedef struct {
    uint32_t  start;
    uint32_t  end;
    uint8_t   marker_len;
    tlv_io_t  io;
} tlv_db_t;

typedef uint16_t tlv_tag_t;

typedef struct {
    uint8_t  data_sz;
    uint8_t  pad_sz;
} tlv_pld_info_t;

typedef struct {
    tlv_tag_t      tag;
    tlv_pld_info_t info;
} tlv_item_hdr_t;


int tlv_read_marker(const tlv_db_t *db, uint8_t *buf, uint8_t sz);

int tlv_write_marker(tlv_db_t *db, const uint8_t *buf, uint8_t sz);

int tlv_fetch(const tlv_db_t *db, tlv_tag_t tag, uint8_t *buf, uint16_t want);

int tlv_find(const tlv_db_t *db, tlv_tag_t tag, tlv_item_hdr_t *out_hdr);

int tlv_store(tlv_db_t *db, tlv_tag_t tag, const uint8_t *data, uint16_t sz);

#endif
