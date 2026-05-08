/********************************************************************************************************
 * @file    sid_mfg_hex_parsers.h
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
#include "tlv.h"
#include <sid_pal_mfg_store_ifc.h>

#define MFG_FLAGS_TYPE_ID SID_PAL_MFG_STORE_VALUE_MAX - 1
struct mfg_flags {
    uint8_t unused_bits : 6;
    uint8_t keys_in_psa : 1;
    uint8_t initialized : 1;
    uint8_t unused[3];
};

#define MFG_HEADER_MAGIC "SID0"
#define MFG_HEADER_MAGIC_SIZE sizeof(MFG_HEADER_MAGIC) - 1
#define REPORTED_VERSION SID_PAL_MFG_STORE_TLV_VERSION

#define INVALID_VERSION 0xFFFFFFFF
struct mfg_header {
    uint8_t magic_string[MFG_HEADER_MAGIC_SIZE];
    uint8_t raw_version[SID_PAL_MFG_STORE_VERSION_SIZE];
};

/**
 * @brief Parse content of the manufacturing partition v8, and write it as tlv.
 * The TLV will replace raw manufacturing partition
 * 
 * @param tlv [IN/OUT] configuration for tlv
 * @return int 0 on success, -ERRNO on error
 */
int parse_mfg_raw_tlv(tlv_ctx *tlv);

/**
 * @brief Parse content of the manufacturing partition v7, and write it as tlv.
 * The TLV will replace raw manufacturing partition
 * 
 * @param tlv [IN/OUT] configuration for tlv
 * @return int 0 on success, -ERRNO on error
 */
int parse_mfg_const_offsets(tlv_ctx *tlv);
