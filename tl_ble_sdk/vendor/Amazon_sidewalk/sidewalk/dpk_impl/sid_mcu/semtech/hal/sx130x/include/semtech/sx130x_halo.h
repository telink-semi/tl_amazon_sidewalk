/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef __SX130X_HALO_H__
#define __SX130X_HALO_H__

#include <loragw_hal.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum sx130x_status_e
{
    SX130X_STATUS_OK    = 0,
    SX130X_STATUS_ERROR = 3,
} sx130x_status_t;

/**
 * @brief SX130X packet types enumeration definition
 */
typedef enum sx130x_pkt_types_e
{
    SX130X_PKT_TYPE_GFSK = 0x00,
    SX130X_PKT_TYPE_LORA = 0x01,
} sx130x_pkt_type_t;

/**
 * @brief SX130X power amplifier ramp-up timings enumeration definition
 */
typedef enum sx130x_ramp_time_e
{
    SX130X_RAMP_10_US   = 0x00,
    SX130X_RAMP_20_US   = 0x01,
    SX130X_RAMP_40_US   = 0x02,
    SX130X_RAMP_80_US   = 0x03,
    SX130X_RAMP_200_US  = 0x04,
    SX130X_RAMP_800_US  = 0x05,
    SX130X_RAMP_1700_US = 0x06,
    SX130X_RAMP_3400_US = 0x07,
} sx130x_ramp_time_t;

/**
 * @brief SX130X power amplifier configuration parameters structure definition
 */
typedef struct sx130x_pa_cfg_params_s
{
    uint8_t pa_duty_cycle;
    uint8_t hp_max;
    uint8_t device_sel;
    uint8_t pa_lut;
} sx130x_pa_cfg_params_t;

/**
 * @brief SX130X GFSK modulation shaping enumeration definition
 */
typedef enum sx130x_gfsk_mod_shapes_e
{
    SX130X_GFSK_MOD_SHAPE_OFF   = 0x00,
    SX130X_GFSK_MOD_SHAPE_BT_03 = 0x08,
    SX130X_GFSK_MOD_SHAPE_BT_05 = 0x09,
    SX130X_GFSK_MOD_SHAPE_BT_07 = 0x0A,
    SX130X_GFSK_MOD_SHAPE_BT_1  = 0x0B,
} sx130x_gfsk_mod_shapes_t;

/**
 * @brief SX130X GFSK Rx bandwidth enumeration definition
 */
typedef enum sx130x_gfsk_bw_e
{
    SX130X_GFSK_BW_4800   = 0x1F,
    SX130X_GFSK_BW_5800   = 0x17,
    SX130X_GFSK_BW_7300   = 0x0F,
    SX130X_GFSK_BW_9700   = 0x1E,
    SX130X_GFSK_BW_11700  = 0x16,
    SX130X_GFSK_BW_14600  = 0x0E,
    SX130X_GFSK_BW_19500  = 0x1D,
    SX130X_GFSK_BW_23400  = 0x15,
    SX130X_GFSK_BW_29300  = 0x0D,
    SX130X_GFSK_BW_39000  = 0x1C,
    SX130X_GFSK_BW_46900  = 0x14,
    SX130X_GFSK_BW_58600  = 0x0C,
    SX130X_GFSK_BW_78200  = 0x1B,
    SX130X_GFSK_BW_93800  = 0x13,
    SX130X_GFSK_BW_117300 = 0x0B,
    SX130X_GFSK_BW_156200 = 0x1A,
    SX130X_GFSK_BW_187200 = 0x12,
    SX130X_GFSK_BW_234300 = 0x0A,
    SX130X_GFSK_BW_312000 = 0x19,
    SX130X_GFSK_BW_373600 = 0x11,
    SX130X_GFSK_BW_467000 = 0x09,
} sx130x_gfsk_bw_t;

/**
 * @brief SX130X GFSK modulation parameters structure definition
 */
typedef struct sx130x_mod_params_gfsk_s
{
    uint32_t                 br_in_bps;
    uint32_t                 fdev_in_hz;
    sx130x_gfsk_mod_shapes_t mod_shape;
    sx130x_gfsk_bw_t         bw_dsb_param;
} sx130x_mod_params_gfsk_t;

/**
 * @brief SX130X LoRa spreading factor enumeration definition
 */
typedef enum sx130x_lora_sf_e
{
    SX130X_LORA_SF5  = 0x05,
    SX130X_LORA_SF6  = 0x06,
    SX130X_LORA_SF7  = 0x07,
    SX130X_LORA_SF8  = 0x08,
    SX130X_LORA_SF9  = 0x09,
    SX130X_LORA_SF10 = 0x0A,
    SX130X_LORA_SF11 = 0x0B,
    SX130X_LORA_SF12 = 0x0C,
} sx130x_lora_sf_t;

/**
 * @brief SX130X LoRa bandwidth enumeration definition
 */
typedef enum sx130x_lora_bw_e
{
    SX130X_LORA_BW_500 = 6,
    SX130X_LORA_BW_250 = 5,
    SX130X_LORA_BW_125 = 4,
    SX130X_LORA_BW_062 = 3,
    SX130X_LORA_BW_041 = 10,
    SX130X_LORA_BW_031 = 2,
    SX130X_LORA_BW_020 = 9,
    SX130X_LORA_BW_015 = 1,
    SX130X_LORA_BW_010 = 8,
    SX130X_LORA_BW_007 = 0,
} sx130x_lora_bw_t;

/**
 * @brief SX130X LoRa coding rate enumeration definition
 */
typedef enum sx130x_lora_cr_e
{
    SX130X_LORA_CR_4_5    = 0x01,
    SX130X_LORA_CR_4_6    = 0x02,
    SX130X_LORA_CR_4_7    = 0x03,
    SX130X_LORA_CR_4_8    = 0x04,
    SX130X_LORA_CR_4_5_LI = 0x05,
    SX130X_LORA_CR_4_6_LI = 0x06,
    SX130X_LORA_CR_4_8_LI = 0x07,
} sx130x_lora_cr_t;

/**
 * @brief SX130X LoRa modulation parameters structure definition
 */
typedef struct sx130x_mod_params_lora_s
{
    sx130x_lora_sf_t sf;    //!< LoRa Spreading Factor
    sx130x_lora_bw_t bw;    //!< LoRa Bandwidth
    sx130x_lora_cr_t cr;    //!< LoRa Coding Rate
    uint8_t          ldro;  //!< Low DataRate Optimization configuration
} sx130x_mod_params_lora_t;

/**
 * @brief SX130X GFSK preamble length Rx detection size enumeration definition
 */
typedef enum sx130x_gfsk_pbl_det_e
{
    SX130X_GFSK_PBL_DET_OFF     = 0x00,
    SX130X_GFSK_PBL_DET_08_BITS = 0x04,
    SX130X_GFSK_PBL_DET_16_BITS = 0x05,
    SX130X_GFSK_PBL_DET_24_BITS = 0x06,
    SX130X_GFSK_PBL_DET_32_BITS = 0x07,
} sx130x_gfsk_pbl_det_t;

/**
 * @brief SX130X GFSK address filtering configuration enumeration definition
 */
typedef enum sx130x_gfsk_addr_cmp_e
{
    SX130X_GFSK_ADDR_CMP_FILT_OFF        = 0x00,
    SX130X_GFSK_ADDR_CMP_FILT_NODE       = 0x01,
    SX130X_GFSK_ADDR_CMP_FILT_NODE_BROAD = 0x02,
} sx130x_gfsk_addr_cmp_t;

/**
 * @brief SX130X GFSK packet length enumeration definition
 */
typedef enum sx130x_gfsk_pkt_len_modes_e
{
    SX130X_GFSK_PKT_FIX_LEN = 0x00,  //!< The packet length is known on both sides, no header included
    SX130X_GFSK_PKT_VAR_LEN = 0x01,  //!< The packet length is variable, header included
} sx130x_gfsk_pkt_len_modes_t;

/**
 * @brief SX130X GFSK CRC type enumeration definition
 */
typedef enum sx16x_gfsk_crc_types_e
{
    SX130X_GFSK_CRC_OFF         = 0x01,
    SX130X_GFSK_CRC_1_BYTE      = 0x00,
    SX130X_GFSK_CRC_2_BYTES     = 0x02,
    SX130X_GFSK_CRC_1_BYTE_INV  = 0x04,
    SX130X_GFSK_CRC_2_BYTES_INV = 0x06,
} sx130x_gfsk_crc_types_t;

/**
 * @brief SX130X GFSK whitening control enumeration definition
 */
typedef enum sx130x_gfsk_dc_free_e
{
    SX130X_GFSK_DC_FREE_OFF       = 0x00,
    SX130X_GFSK_DC_FREE_WHITENING = 0x01,
} sx130x_gfsk_dc_free_t;

/**
 * @brief SX130X LoRa packet length enumeration definition
 */
typedef enum sx130x_lora_pkt_len_modes_e
{
    SX130X_LORA_PKT_EXPLICIT = 0x00,  //!< Header included in the packet
    SX130X_LORA_PKT_IMPLICIT = 0x01,  //!< Header not included in the packet
} sx130x_lora_pkt_len_modes_t;

/**
 * @brief SX130X LoRa packet parameters structure definition
 */
typedef struct sx130x_pkt_params_lora_s
{
    uint16_t                    pbl_len_in_symb;   //!< Preamble length in symbols
    sx130x_lora_pkt_len_modes_t hdr_type;          //!< Header type
    uint8_t                     pld_len_in_bytes;  //!< Payload length in bytes
    bool                        crc_is_on;         //!< CRC activation
    bool                        invert_iq_is_on;   //!< IQ polarity setup
} sx130x_pkt_params_lora_t;

/**
 * @brief SX130X GFSK packet parameters structure definition
 */
typedef struct sx130x_pkt_params_gfsk_s
{
    uint16_t                    pbl_len_in_bits;        //!< Preamble length in bits
    sx130x_gfsk_pbl_det_t       pbl_min_det;            //!< Preamble detection length
    uint8_t                     sync_word_len_in_bits;  //!< Sync word length in bits
    sx130x_gfsk_addr_cmp_t      addr_cmp;               //!< Address filtering configuration
    sx130x_gfsk_pkt_len_modes_t hdr_type;               //!< Header type
    uint8_t                     pld_len_in_bytes;       //!< Payload length in bytes
    sx130x_gfsk_crc_types_t     crc_type;               //!< CRC type configuration
    sx130x_gfsk_dc_free_t       dc_free;                //!< Whitening configuration
} sx130x_pkt_params_gfsk_t;

/**
 * @brief SX130X LoRa CAD number of symbols enumeration definition
 *
 * @note Represents the number of symbols to be used for a CAD operation
 */
typedef enum sx130x_lora_cad_symbs_e
{
    SX130X_LORA_CAD_01_SYMB = 0x00,
    SX130X_LORA_CAD_02_SYMB = 0x01,
    SX130X_LORA_CAD_04_SYMB = 0x02,
    SX130X_LORA_CAD_08_SYMB = 0x03,
    SX130X_LORA_CAD_16_SYMB = 0x04,
} sx130x_lora_cad_symbs_t;

/**
 * @brief SX130X LoRa CAD exit modes enumeration definition
 *
 * @note Represents the action to be performed after a CAD is done
 */
typedef enum sx130x_lora_cad_exit_modes_e
{
    SX130X_LORA_CAD_ONLY = 0x00,
    SX130X_LORA_CAD_RX   = 0x01,
    SX130X_LORA_CAD_LBT  = 0x10,
} sx130x_lora_cad_exit_modes_t;

/**
 * @brief SX130X LoRa CAD parameters structure definition
 */
typedef struct sx130x_lora_cad_param_s
{
    sx130x_lora_cad_symbs_t      cad_symb_nb;    //!< CAD number of symbols
    uint8_t                      cad_det_peak;   //!< CAD peak detection
    uint8_t                      cad_det_min;    //!< CAD minimum detection
    sx130x_lora_cad_exit_modes_t cad_exit_mode;  //!< CAD exit mode
    uint32_t                     cad_timeout;    //!< CAD timeout value
} sx130x_lora_cad_params_t;

int32_t sx130x_set_platform(void);

void sx130x_radio_process(void);

int32_t sx130x_restart(void);

int32_t sx130x_set_txpower(int8_t powerLevel);

int32_t sx130x_set_rf_freq(const uint32_t freq_in_hz);

int32_t sx130x_set_standby(const void *context);

int32_t sx130x_set_tx_payload(const uint8_t *buffer, uint8_t size);

int32_t sx130x_set_tx(const void *context, const uint32_t timeout);

int32_t sx130x_set_tx_cw(const void *context);

int32_t sx130x_set_rx(const void *context, const uint32_t timeout);

int32_t sx130x_get_rssi_inst(const void *context, int16_t *rssi_in_dbm);

int32_t sx130x_reset(const void *context);

int32_t sx130x_set_lora_sync_word(const void *context, const uint8_t sync_word);

int32_t sx130x_set_lora_mod_params(const void *context, const sx130x_mod_params_lora_t *params);

int32_t sx130x_set_lora_pkt_params(const void *context, const sx130x_pkt_params_lora_t *params);

int32_t sx130x_set_cad_params(const void *context, const sx130x_lora_cad_params_t *params);

uint32_t sx130x_get_lora_bw_in_hz(sx130x_lora_bw_t bw);

uint32_t sx130x_get_lora_time_on_air_numerator(const sx130x_pkt_params_lora_t *pkt_p,
                                               const sx130x_mod_params_lora_t *mod_p);

uint32_t sx130x_get_lora_cad_duration_microsecs(uint8_t symbol ,
                                                const sx130x_mod_params_lora_t *mod_p);

uint32_t sx130x_get_lora_time_on_air_in_ms(const sx130x_pkt_params_lora_t *pkt_p,
                                           const sx130x_mod_params_lora_t *mod_p);

#ifdef __cplusplus
}
#endif

#endif
