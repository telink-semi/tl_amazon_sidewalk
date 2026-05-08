/********************************************************************************************************
 * @file    tlv.c
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
#include "tlv.h"


static inline uint16_t calc_pad(uint16_t v)
{
    return (TLV_ALIGN - (v & (TLV_ALIGN - 1))) & (TLV_ALIGN - 1);
}


static tlv_item_hdr_t decode_hdr(const uint8_t b[4])
{
    tlv_item_hdr_t h;
    h.tag        = (uint16_t)b[0] << 8 | b[1];
    h.info.pad_sz = b[2];
    h.info.data_sz = b[3];
    return h;
}

static void encode_hdr(tlv_item_hdr_t h, uint8_t b[4])
{
    b[0] = h.tag >> 8;
    b[1] = h.tag & 0xFF;
    b[2] = h.info.pad_sz;
    b[3] = h.info.data_sz;
}

static uint32_t find_free(const tlv_db_t *db)
{
    uint32_t off = db->start + db->marker_len;
    while (off + TLV_HDR_LEN <= db->end) {
        uint8_t hb[TLV_HDR_LEN];
        if (db->io.read(db->io.ctx, off, hb, sizeof(hb)) != 0)
            return db->end;
        tlv_item_hdr_t h = decode_hdr(hb);
        if (h.tag == TLV_EOF_TAG || h.info.data_sz == 0)
            return off;
        off += TLV_HDR_LEN + h.info.data_sz + h.info.pad_sz;
    }
    return db->end;
}

int tlv_find(const tlv_db_t *db, tlv_tag_t tag, tlv_item_hdr_t *out_hdr)
{
    if (!db || !db->io.read)
        return -EINVAL;

    uint32_t off = db->start + db->marker_len;
    while (off + TLV_HDR_LEN <= db->end) {
        uint8_t buf[TLV_HDR_LEN];
        if (db->io.read(db->io.ctx, off, buf, sizeof(buf)) != 0)
            return -EIO;
        tlv_item_hdr_t h = decode_hdr(buf);
        if (h.tag == tag) {
            if (out_hdr) *out_hdr = h;
            return 0;
        }
        uint32_t step = TLV_HDR_LEN + h.info.data_sz + h.info.pad_sz;
        off += step;
    }
    return -ENODATA;
}
int tlv_fetch(const tlv_db_t *db, tlv_tag_t tag, uint8_t *buf, uint16_t want)
{
    if (!db || !db->io.read)
        return -EINVAL;

    uint32_t off = db->start + db->marker_len;
    while (off + TLV_HDR_LEN <= db->end) {
        uint8_t hb[TLV_HDR_LEN];
        if (db->io.read(db->io.ctx, off, hb, sizeof(hb)) != 0)
            return -EIO;
        tlv_item_hdr_t h = decode_hdr(hb);
        uint32_t data_off = off + TLV_HDR_LEN;
        if (h.tag == tag) {
            uint16_t total = h.info.data_sz + h.info.pad_sz;
            if (want > total)
                return -ENOMEM;
            if (data_off + want > db->end)
                return -ENODATA;
            return db->io.read(db->io.ctx, data_off, buf, want);
        }
        off += TLV_HDR_LEN + h.info.data_sz + h.info.pad_sz;
    }
    return -ENODATA;
}


int tlv_store(tlv_db_t *db, tlv_tag_t tag, const uint8_t *data, uint16_t sz)
{
    if (!db || !db->io.read || !db->io.write)
        return -EINVAL;

    uint32_t free_off = find_free(db);
    uint8_t  pad      = calc_pad(sz);
    uint32_t need     = TLV_HDR_LEN + sz + pad;
    if (free_off + need > db->end)
        return -ENOMEM;

    tlv_item_hdr_t h = { .tag = tag, .info = { .data_sz = sz, .pad_sz = pad } };
    uint8_t hb[TLV_HDR_LEN];
    encode_hdr(h, hb);
    if (db->io.write(db->io.ctx, free_off, hb, sizeof(hb)) != 0)
        return -EIO;
    free_off += sizeof(hb);

    uint8_t blk[TLV_ALIGN];
    for (uint16_t done = 0; done < sz + pad; ) {
        memset(blk, TLV_PAD_BYTE, sizeof(blk));
        uint16_t chunk = sz - done;
        if (chunk > sizeof(blk)) chunk = sizeof(blk);
        if (chunk && data) memcpy(blk, data + done, chunk);
        if (db->io.write(db->io.ctx, free_off, blk, sizeof(blk)) != 0)
            return -EIO;
        free_off += sizeof(blk);
        done     += sizeof(blk);
    }
    return 0;
}

int tlv_read_marker(const tlv_db_t *db, uint8_t *buf, uint8_t sz)
{
    if (!db || !db->io.read)
        return -EINVAL;
    if (sz != db->marker_len)
        return -ENOMEM;
    return db->io.read(db->io.ctx, db->start, buf, sz);
}

int tlv_write_marker(tlv_db_t *db, const uint8_t *buf, uint8_t sz)
{
    if (!db || !db->io.write)
        return -EINVAL;
    if (sz != db->marker_len)
        return -ENOMEM;
    return db->io.write(db->io.ctx, db->start, buf, sz);
}
