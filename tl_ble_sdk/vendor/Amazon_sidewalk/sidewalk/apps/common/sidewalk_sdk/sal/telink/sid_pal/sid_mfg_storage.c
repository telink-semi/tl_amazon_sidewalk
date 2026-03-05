/********************************************************************************************************
 * @file    sid_mfg_storage.c
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
/** @file sid_mfg_storage.c
 *  @brief Sidewalk MFG storage.
 */
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "sid_ble_adapter.h"
#include <sid_pal_mfg_store_ifc.h>
#include <sid_error.h>
#include <tlv.h>
#include "app_nv.h"
#include "tlv_storage_impl.h"

#define MFG_STORE_TLV_TAG_EMPTY 0xFFFF

#define FLASH_MEM_CHUNK (128)

#ifndef DEV_ID_REG

#define DEV_ID_REG    0x1715  //todo!!!!!

#endif /* DEV_ID_REG */

#define SID_SMSN_SIZE 32
#define APP_MFG_CFG_FLASH_SIZE 1024*2
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

_attribute_ble_data_retention_ static uint32_t sid_mfg_version = INVALID_VERSION;
_attribute_ble_data_retention_ static uint32_t sid_mfg_start_addr = CFG_ADR_SIDEWALK_MFG_2M_FLASH;

static  tlv_db_t  tlv_flash;

void sid_mfg_set_start_addr(uint32_t  addr)
{
    sid_mfg_start_addr = addr;
}

uint32_t sid_mfg_get_start_addr(void)
{
    return sid_mfg_start_addr;
}

uint32_t sid_mfg_get_end_addr(void)
{
    return (sid_mfg_start_addr + APP_MFG_CFG_FLASH_SIZE);
}


int refra_mfg_data(tlv_db_t  *tlv)
{
    if (tlv->end <= tlv->start) {
        return -EINVAL;
    }

    uint32_t size = tlv->end - tlv->start;

    uint8_t *ram_tlv_data = sid_malloc(size);
    if (ram_tlv_data == NULL) {
        return -ENOMEM;
    }
    memset(ram_tlv_data, 0xff, size);

    tlv_db_t  ram_tlv = {
            .start = 0,
            .end = size,
            .marker_len  = tlv->marker_len,
            .io = { .ctx = ram_tlv_data,
                          .read = tlv_storage_ram_read,
                          .write = tlv_storage_ram_write } };

    uint32_t offset = tlv->start + tlv->marker_len;

    struct mfg_header mfg_header = { .magic_string = MFG_HEADER_MAGIC,
                     .raw_version = { 0, 0, 0, REPORTED_VERSION } };

    int ret =
        tlv_write_marker(&ram_tlv, (uint8_t *)&mfg_header, sizeof(struct mfg_header));
    if (ret != 0) {
        TL_LOG_E("Failed to write marker before mfg data");
        sid_free(ram_tlv_data);
        return -EIO;
    }

    uint8_t payload_buffer[CONFIG_SIDEWALK_MFG_PARSER_MAX_ELEMENT_SIZE] = { 0 };
    for (; offset < tlv->end;) {
        uint8_t key[2] = { 0 };
        uint8_t size[2] = { 0 };
        int ret = tlv->io.read(tlv->io.ctx, offset, key, sizeof(key));
        if (ret != 0) {
            TL_LOG_E("Failed to read data");
            sid_free(ram_tlv_data);
            return -EIO;
        }
        offset += 2;
        uint16_t key_decoded = (key[0] << 8) + key[1];
        if (key_decoded == MFG_STORE_TLV_TAG_EMPTY) {
            break;
        }
        ret = tlv->io.read(tlv->io.ctx, offset, size, sizeof(size));
        if (ret != 0) {
            TL_LOG_E("Failed to read data");
            sid_free(ram_tlv_data);
            return -EIO;
        }
        offset += 2;
        uint16_t size_decoded = (size[0] << 8) | size[1];
        ret = tlv->io.read(tlv->io.ctx, offset, payload_buffer,
                         size_decoded);
        if (ret != 0) {
            TL_LOG_E("Failed to read data");
            sid_free(ram_tlv_data);
            return -EIO;
        }
        offset += size_decoded;

        ret = tlv_store(&ram_tlv, key_decoded, payload_buffer, size_decoded);
        if (ret != 0) {
            TL_LOG_E("Failed to write data");
            sid_free(ram_tlv_data);
            return -EIO;
        }
    }

    struct mfg_flags flags = {
        .initialized = 1,
    };
    ret = tlv_store(&ram_tlv, MFG_FLAGS_TYPE_ID, (uint8_t *)&flags, sizeof(flags));
    if (ret != 0) {
        TL_LOG_E("Failed to write data");
        sid_free(ram_tlv_data);
        return -EIO;
    }

    ret = tlv->io.erase(tlv->io.ctx, tlv->start, size);
    if (ret != 0) {
        TL_LOG_E("Failed to erase flash storage");
        sid_free(ram_tlv_data);
        return -EIO;
    }

    ret = tlv->io.write(tlv->io.ctx, tlv->start, ram_tlv_data, size);
    if (ret != 0) {
        TL_LOG_E("Failed to write parsed tlv data to flash");
    }
exit:
    sid_free(ram_tlv_data);
    return ret;
}

void sid_pal_mfg_store_init(sid_pal_mfg_store_region_t mfg_store_region)
{
    struct mfg_header header = { 0 };
    bool need_to_parse = false;
    int err = 0;

    tlv_flash = (tlv_db_t ){ .io = { .write = tlv_storage_flash_write,
                         .erase = tlv_storage_flash_erase,
                         .read = tlv_storage_flash_read,
                         .ctx = (void *)NULL},
                   .start = mfg_store_region.addr_start,
                   .end = mfg_store_region.addr_end,
                   .marker_len = sizeof(struct mfg_header) };

    /* Read mfg header */
    err = tlv_read_marker(&tlv_flash, (uint8_t *)&header, sizeof(header));
    if (err || strncmp(header.magic_string, MFG_HEADER_MAGIC, strlen(MFG_HEADER_MAGIC)) != 0) {
        if (err) {
            TL_LOG_E("Failed to read mfg start marker errno %d", err);
        }
#if CONFIG_SIDEWALK_ON_DEV_CERT
        TL_LOG_I("Please perform on_device_certification and reboot");
#endif
        return;
    }

    /* Check mfg version */
    sid_mfg_version = header.raw_version[3] + (header.raw_version[2] << 8) +
              (header.raw_version[1] << 16) + ((header.raw_version[0]) << 24);

    if (sid_mfg_version == SID_PAL_MFG_STORE_TLV_VERSION) {
        /* Check mfg flags */
        struct mfg_flags flags = {};
        err = tlv_fetch(&tlv_flash, MFG_FLAGS_TYPE_ID, (uint8_t *)&flags, sizeof(flags));
        if (err == -ENODATA) {
            need_to_parse = true;
        } else if (err) {
            TL_LOG_E("Failed to read mfg data errno: %d", err);
            return;
        }

        if (!flags.initialized) {
            need_to_parse = true;
        }
        if (flags.keys_in_psa) {
            TL_LOG_E("Failed to init mfg storage. No secure key storage support");
            TL_LOG_I("Rebuild with CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE=y or Flash mfg data again");
            return;
        }
    } else {
        need_to_parse = true;
    }

    if (need_to_parse) {
        /* Save mfg data in desired version format */
        TL_LOG_I("Need to parse mfg data %d",sid_mfg_version);
        err = refra_mfg_data(&tlv_flash);
        if (err) {
            TL_LOG_E("Failed parsing mfg data errno %d", err);
            return;
        }
        TL_LOG_I("Successfully parsed mfg data");
        sid_mfg_version = SID_PAL_MFG_STORE_TLV_VERSION;
    }
    uint8_t smsn_buffer[SID_SMSN_SIZE] = { 0 };
    sid_pal_mfg_store_read(SID_PAL_MFG_STORE_SMSN, smsn_buffer, SID_SMSN_SIZE);
    if (memcmp(smsn_buffer, (const uint8_t[SID_SMSN_SIZE]){ 0 }, SID_SMSN_SIZE) == 0) {
        TL_LOG_E("SMSN is not initialized");
    } else {
        tlkapi_send_string_data(1, "Initialized with SMSN KEY: ", smsn_buffer, SID_SMSN_SIZE);
    }
}

void sid_pal_mfg_store_deinit(void)
{
    memset(&tlv_flash, 0x0, sizeof(tlv_flash));
}

int32_t sid_pal_mfg_store_write(uint16_t value, const uint8_t *buffer, uint16_t length)
{
    if (value == SID_PAL_MFG_STORE_VERSION) {
        if (length != SID_PAL_MFG_STORE_VERSION_SIZE) {
            return -EINVAL;
        }
        struct mfg_header mfg_header = { .magic_string = MFG_HEADER_MAGIC };
        memcpy(mfg_header.raw_version, buffer, SID_PAL_MFG_STORE_VERSION_SIZE);
        return tlv_write_marker(&tlv_flash, (uint8_t *)&mfg_header,
                          sizeof(struct mfg_header));
    }

#if CONFIG_SIDEWALK_MFG_STORAGE_DIAGNOSTIC
    return tlv_store(&tlv_flash, value, buffer, length);
#else
    return (int32_t)SID_ERROR_NOSUPPORT;
#endif
}

void sid_pal_mfg_store_read(uint16_t value, uint8_t *buffer, uint16_t length)
{
    if (value == SID_PAL_MFG_STORE_VERSION) {
        memcpy(buffer, (uint8_t[]){ 0, 0, 0, SID_PAL_MFG_STORE_TLV_VERSION },
               SID_PAL_MFG_STORE_VERSION_SIZE);
        return;
    }
    int ret = tlv_fetch(&tlv_flash, value, buffer, length);
    if (ret != 0) {
        TL_LOG_E("Failed to read tlv type %d with errno %d", value, ret);
    }
}

uint16_t sid_pal_mfg_store_get_length_for_value(uint16_t value)
{
    tlv_item_hdr_t  hdr = {};

    int ret = tlv_find(&tlv_flash, value, &hdr);
    if (ret != 0) {
        TL_LOG_E("Failed to find value %d in MFG storage errno: %d", value, ret);
        return 0;
    }
    return hdr.info.data_sz;
}

int32_t sid_pal_mfg_store_erase(void)
{
#if CONFIG_SIDEWALK_MFG_STORAGE_DIAGNOSTIC
    const size_t mfg_size = tlv_flash.end - tlv_flash.start;
    return tlv_flash.io.erase(tlv_flash.io.ctx, tlv_flash.start,
                        mfg_size);
#else
    return (int32_t)SID_ERROR_NOSUPPORT;
#endif /* CONFIG_SIDEWALK_MFG_STORAGE_DIAGNOSTIC */
}

bool sid_pal_mfg_store_is_empty(void)
{
#if CONFIG_SIDEWALK_MFG_STORAGE_DIAGNOSTIC
    // read header, if failed to read magic, it is empty

    uint8_t empty_flash_mem[FLASH_MEM_CHUNK];
    uint8_t tmp_buff[FLASH_MEM_CHUNK];
    size_t length = sizeof(tmp_buff);
    int rc;
    const struct flash_parameters *flash_params;

    memset(empty_flash_mem, 0xFF, sizeof(empty_flash_mem));
    for (off_t offset = tlv_flash.start; offset < tlv_flash.end;
         offset += length) {
        if ((offset + length) > tlv_flash.end) {
            length = tlv_flash.end - offset;
        }
        flash_read_page(offset,length,tmp_buff);
        if (0 != memcmp(empty_flash_mem, tmp_buff, length)) {
            return false;
        }
    }
    return true;
#else
    TL_LOG_W("The sid_pal_mfg_store_is_empty function is not enabled.");
    return false;
#endif /* CONFIG_SIDEWALK_MFG_STORAGE_DIAGNOSTIC */
}

bool sid_pal_mfg_store_is_tlv_support(void)
{
    return true;
}

uint32_t sid_pal_mfg_store_get_version(void)
{
    return sid_mfg_version;
}

/* Functions specific to Sidewalk with special handling */

bool sid_pal_mfg_store_dev_id_get(uint8_t dev_id[SID_PAL_MFG_STORE_DEVID_SIZE])
{
    uint32_t mcu_devid = DEV_ID_REG;
    dev_id[0] = 0xBF;
    swap32(mcu_devid,mcu_devid);
    memcpy(&dev_id[1], &mcu_devid, sizeof(mcu_devid));
    return true;
}

bool sid_pal_mfg_store_serial_num_get(uint8_t serial_num[SID_PAL_MFG_STORE_SERIAL_NUM_SIZE])
{
    int ret = tlv_fetch(&tlv_flash, SID_PAL_MFG_STORE_SERIAL_NUM, serial_num,
               SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);
    return ret == 0;
}

void sid_pal_mfg_store_apid_get(uint8_t apid[SID_PAL_MFG_STORE_APID_SIZE])
{
    sid_pal_mfg_store_read(SID_PAL_MFG_STORE_APID, apid, SID_PAL_MFG_STORE_APID_SIZE);
}

void sid_pal_mfg_store_app_pub_key_get(uint8_t app_pub[SID_PAL_MFG_STORE_APP_PUB_ED25519_SIZE])
{
    sid_pal_mfg_store_read(SID_PAL_MFG_STORE_APP_PUB_ED25519, app_pub,
                   SID_PAL_MFG_STORE_APP_PUB_ED25519_SIZE);
}


