/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.  This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <sid_pal_radio_ifc.h>
#include <semtech_radio_ifc.h>

#include <sx130x_radio.h>

#include <sid_utils.h>

int32_t semtech_radio_set_trim_cap_val(uint16_t trim)
{
    return set_radio_sx130x_trim_cap_val(trim);
}

uint16_t semtech_radio_get_trim_cap_val(void)
{
    const halo_drv_semtech_ctx_t *drv_ctx = sx130x_get_drv_ctx();
    return drv_ctx->trim;
}

int32_t semtech_radio_get_tx_power_range(int8_t *max_tx_power, int8_t *min_tx_power)
{
    const halo_drv_semtech_ctx_t *drv_ctx = sx130x_get_drv_ctx();

    if (drv_ctx->sx130x_cfg->id == SEMTECH_ID_SX1302) {
        *max_tx_power = drv_ctx->sx130x_cfg->max_tx_pwr;
        *min_tx_power = drv_ctx->sx130x_cfg->min_tx_pwr;
        return RADIO_ERROR_NONE;
    } else if (drv_ctx->sx130x_cfg->id == SEMTECH_ID_SX1303) {
        *max_tx_power = drv_ctx->sx130x_cfg->max_tx_pwr;
        *min_tx_power = drv_ctx->sx130x_cfg->min_tx_pwr;
        return RADIO_ERROR_NONE;
    }

    return RADIO_ERROR_INVALID_PARAMS;
}

int32_t semtech_radio_get_pa_config(semtech_radio_pa_cfg_t *cfg)
{
    if (!cfg) {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    radio_sx130x_pa_cfg_t cur_cfg;

    get_radio_sx130x_pa_config(&cur_cfg);

    cfg->pa_duty_cycle = cur_cfg.pa_duty_cycle;
    cfg->hp_max = cur_cfg.hp_max;
    cfg->device_sel = cur_cfg.device_sel;
    cfg->pa_lut = cur_cfg.pa_lut;
    cfg->tx_power = cur_cfg.tx_power;
    cfg->ramp_time = cur_cfg.ramp_time;

    return RADIO_ERROR_NONE;
}

const uint8_t lora_sf[] = { SX130X_LORA_SF5,  SX130X_LORA_SF6,  SX130X_LORA_SF7,
                            SX130X_LORA_SF8,  SX130X_LORA_SF9,  SX130X_LORA_SF10,
                            SX130X_LORA_SF11, SX130X_LORA_SF12};

const uint8_t lora_bw[] = { SX130X_LORA_BW_500, SX130X_LORA_BW_250, SX130X_LORA_BW_125,
                            SX130X_LORA_BW_062, SX130X_LORA_BW_041, SX130X_LORA_BW_031,
                            SX130X_LORA_BW_020, SX130X_LORA_BW_015, SX130X_LORA_BW_010,
                            SX130X_LORA_BW_007};

const uint8_t lora_cr[] = { SX130X_LORA_CR_4_5, SX130X_LORA_CR_4_6, SX130X_LORA_CR_4_7,
                            SX130X_LORA_CR_4_8, SX130X_LORA_CR_4_5_LI, SX130X_LORA_CR_4_6_LI,
                            SX130X_LORA_CR_4_8_LI};

const uint8_t lora_cad_symbol[] = { SX130X_LORA_CAD_01_SYMB, SX130X_LORA_CAD_02_SYMB,
                                    SX130X_LORA_CAD_04_SYMB, SX130X_LORA_CAD_08_SYMB,
                                    SX130X_LORA_CAD_16_SYMB};

const uint8_t lora_cad_exit_mode[] = {SX130X_LORA_CAD_ONLY, SX130X_LORA_CAD_RX, SX130X_LORA_CAD_LBT};

#define LORA_SF_NUM countof(lora_sf)
#define LORA_BW_NUM countof(lora_bw)
#define LORA_CR_NUM countof(lora_cr)
#define LORA_CAD_SYMBOL_NUM countof(lora_cad_symbol)
#define LORA_CAD_EXIT_MODE_NUM countof(lora_cad_exit_mode)


int32_t semtech_radio_get_lora_sf(uint8_t idx, uint8_t *sf)
{
    if (idx >= LORA_SF_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *sf = lora_sf[idx];
    return RADIO_ERROR_NONE;
}

int16_t semtech_radio_get_lora_sf_idx(uint8_t sf)
{
    for (int i = 0; i < LORA_SF_NUM; i++) {
        if (lora_sf[i] == sf)
            return i;
    }
    return -1;
}


int32_t semtech_radio_get_lora_bw(uint8_t idx, uint8_t *bw)
{
    if (idx >= LORA_BW_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *bw = lora_bw[idx];
    return RADIO_ERROR_NONE;
}

int16_t semtech_radio_get_lora_bw_idx(uint8_t bw)
{
    for (int i = 0; i < LORA_BW_NUM; i++) {
        if (lora_bw[i] == bw) {
            return i;
        }
    }
    return -1;
}

int32_t semtech_radio_get_lora_cr(uint8_t idx, uint8_t *cr)
{
    if (idx >= LORA_CR_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *cr = lora_cr[idx];
    return RADIO_ERROR_NONE;
}

int16_t semtech_radio_get_lora_cr_idx(uint8_t cr)
{
    for (int i = 0; i < LORA_CR_NUM; i++) {
        if (lora_cr[i] == cr) {
            return i;
        }
    }
    return -1;
}

int32_t semtech_radio_get_lora_cad_symbol(uint8_t idx, uint8_t *sym)
{
    if (idx >= LORA_CAD_SYMBOL_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *sym = lora_cad_symbol[idx];
    return RADIO_ERROR_NONE;
}

int16_t semtech_radio_get_lora_cad_symbol_idx(uint8_t sym)
{
    for (int i = 0; i < LORA_CAD_SYMBOL_NUM; i++) {
        if (lora_cad_symbol[i] == sym) {
            return i;
        }
    }
    return -1;
}

int32_t semtech_radio_get_lora_cad_exit_mode(uint8_t idx, uint8_t *em)
{
    if (idx >= LORA_CAD_EXIT_MODE_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *em = lora_cad_exit_mode[idx];
    return RADIO_ERROR_NONE;
}

int16_t semtech_radio_get_lora_cad_exit_mode_idx(uint8_t em)
{
    for (int i = 0; i < LORA_CAD_EXIT_MODE_NUM; i++) {
        if (lora_cad_exit_mode[i] == em) {
            return i;
        }
    }
    return -1;
}

const uint8_t fsk_mod_shaping[] = {SX130X_GFSK_MOD_SHAPE_OFF, SX130X_GFSK_MOD_SHAPE_BT_03,
                                   SX130X_GFSK_MOD_SHAPE_BT_05, SX130X_GFSK_MOD_SHAPE_BT_07,
                                   SX130X_GFSK_MOD_SHAPE_BT_1};

const uint8_t fsk_bw[] = {SX130X_GFSK_BW_4800,   SX130X_GFSK_BW_5800,   SX130X_GFSK_BW_7300,
                          SX130X_GFSK_BW_9700,   SX130X_GFSK_BW_11700,  SX130X_GFSK_BW_14600,
                          SX130X_GFSK_BW_19500,  SX130X_GFSK_BW_23400,  SX130X_GFSK_BW_29300,
                          SX130X_GFSK_BW_39000,  SX130X_GFSK_BW_46900,  SX130X_GFSK_BW_58600,
                          SX130X_GFSK_BW_78200,  SX130X_GFSK_BW_93800,  SX130X_GFSK_BW_117300,
                          SX130X_GFSK_BW_156200, SX130X_GFSK_BW_187200, SX130X_GFSK_BW_234300,
                          SX130X_GFSK_BW_312000, SX130X_GFSK_BW_373600, SX130X_GFSK_BW_467000};

const uint8_t fsk_addr_comp[] = {SX130X_GFSK_ADDR_CMP_FILT_OFF, SX130X_GFSK_ADDR_CMP_FILT_NODE,
                                 SX130X_GFSK_ADDR_CMP_FILT_NODE_BROAD};

const uint8_t fsk_preamble_detect[] = {SX130X_GFSK_PBL_DET_OFF, SX130X_GFSK_PBL_DET_08_BITS,
                                       SX130X_GFSK_PBL_DET_16_BITS, SX130X_GFSK_PBL_DET_24_BITS,
                                       SX130X_GFSK_PBL_DET_32_BITS};

const uint8_t fsk_crc_type[] = {SX130X_GFSK_CRC_OFF, SX130X_GFSK_CRC_1_BYTE, SX130X_GFSK_CRC_2_BYTES,
                                SX130X_GFSK_CRC_1_BYTE_INV, SX130X_GFSK_CRC_2_BYTES_INV};

#define FSK_MOD_SHAPING_PARAMS_NUM     countof(fsk_mod_shaping)
#define FSK_BW_NUM                     countof(fsk_bw)
#define FSK_ADDR_COMP_NUM              countof(fsk_addr_comp)
#define FSK_PREAMBLE_DETECT_NUM        countof(fsk_preamble_detect)
#define FSK_CRC_TYPES_NUM              countof(fsk_crc_type)

int32_t semtech_radio_get_fsk_mod_shaping(uint8_t idx, uint8_t *ms)
{
    if (idx >= FSK_MOD_SHAPING_PARAMS_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *ms = fsk_mod_shaping[idx];
    return RADIO_ERROR_NONE;
}

int16_t semtech_radio_get_fsk_mod_shaping_idx(uint8_t ms)
{
    for (int i = 0; i < FSK_MOD_SHAPING_PARAMS_NUM; i++) {
        if (fsk_mod_shaping[i] == ms) {
            return i;
        }
    }
    return -1;
}

int32_t semtech_radio_get_fsk_bw(uint8_t idx, uint8_t *bw)
{
    if (idx >= FSK_BW_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *bw = fsk_bw[idx];
    return RADIO_ERROR_NONE;
}

int16_t semtech_radio_get_fsk_bw_idx(uint8_t bw)
{
    for (int i = 0; i < FSK_BW_NUM; i++) {
        if (fsk_bw[i] == bw) {
            return i;
        }
    }
    return -1;
}

int32_t semtech_radio_get_fsk_addr_comp(uint8_t idx, uint8_t *ac)
{
    if (idx >= FSK_ADDR_COMP_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *ac = fsk_addr_comp[idx];
    return RADIO_ERROR_NONE;
}

int16_t semtech_radio_get_fsk_addr_comp_idx(uint8_t ac)
{
    for (int i = 0; i < FSK_ADDR_COMP_NUM; i++) {
        if (fsk_addr_comp[i] == ac) {
            return i;
        }
    }
    return -1;
}

int32_t semtech_radio_get_fsk_preamble_detect(uint8_t idx, uint8_t *pd)
{
    if (idx >= FSK_PREAMBLE_DETECT_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *pd = fsk_preamble_detect[idx];
    return RADIO_ERROR_NONE;
}

int16_t semtech_radio_get_fsk_preamble_detect_idx(uint8_t pd)
{
    for (int i = 0; i < FSK_PREAMBLE_DETECT_NUM; i++) {
        if (fsk_preamble_detect[i] == pd) {
            return i;
        }
    }
    return -1;
}

int32_t semtech_radio_get_fsk_crc_type(uint8_t idx, uint8_t *crc)
{
    if (idx >= FSK_CRC_TYPES_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *crc = fsk_crc_type[idx];
    return RADIO_ERROR_NONE;
}


int16_t semtech_radio_get_fsk_crc_type_idx(uint8_t crc)
{
    for (int i = 0; i < FSK_CRC_TYPES_NUM; i++) {
        if (fsk_crc_type[i] == crc) {
            return i;
        }
    }
    return -1;
}

