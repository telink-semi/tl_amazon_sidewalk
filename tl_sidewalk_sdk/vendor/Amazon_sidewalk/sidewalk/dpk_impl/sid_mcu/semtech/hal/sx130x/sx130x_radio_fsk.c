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

#include <sx130x_radio.h>
#define SX130X_FSK_PROCESS_DELAY_US_MIN 1000
#define SX130X_FSK_PROCESS_DELAY_US_MAX 5000
#define SX130X_FSK_TX_PROCESS_DELAY_US 1000
#define SX130X_FSK_RX_PROCESS_DELAY_US 1000

int32_t radio_fsk_process_sync_word_detected(halo_drv_semtech_ctx_t *ctx)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}

int32_t radio_fsk_process_rx_done(halo_drv_semtech_ctx_t *drv_ctx, radio_fsk_rx_done_status_t *rx_done_status)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}

sid_pal_radio_data_rate_t sid_pal_radio_fsk_mod_params_to_data_rate(const sid_pal_radio_fsk_modulation_params_t *mp)
{
    return SID_PAL_RADIO_DATA_RATE_INVALID;
}

int32_t sid_pal_radio_fsk_data_rate_to_mod_params(sid_pal_radio_fsk_modulation_params_t *mod_params,
                                                  sid_pal_radio_data_rate_t data_rate)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}

int32_t sid_pal_radio_prepare_fsk_for_rx(sid_pal_radio_fsk_pkt_cfg_t *rx_pkt_cfg)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}

int32_t sid_pal_radio_prepare_fsk_for_tx(sid_pal_radio_fsk_pkt_cfg_t *tx_pkt_cfg)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}

int32_t sid_pal_radio_set_fsk_sync_word(const uint8_t *sync_word, uint8_t sync_word_length)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}

int32_t sid_pal_radio_set_fsk_whitening_seed(uint16_t seed)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}

int32_t sid_pal_radio_set_fsk_modulation_params(const sid_pal_radio_fsk_modulation_params_t *mod_params)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}

int32_t sid_pal_radio_set_fsk_packet_params(const sid_pal_radio_fsk_packet_params_t *packet_params)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}

uint32_t sid_pal_radio_fsk_time_on_air(const sid_pal_radio_fsk_modulation_params_t *mod_params,
                                       const sid_pal_radio_fsk_packet_params_t *packet_params,
                                       uint8_t packetLen)
{
    if (mod_params == NULL || packet_params == NULL) {
        return 0;
    }
    return 0;
}

uint32_t sid_pal_radio_fsk_get_fsk_number_of_symbols(const sid_pal_radio_fsk_modulation_params_t *mod_params,
                                                     uint32_t delay_micro_secs)
{
    return 0;
}

uint32_t sid_pal_radio_get_fsk_tx_process_delay(void)
{
    halo_drv_semtech_ctx_t *drv_ctx = sx130x_get_drv_ctx();
    return (drv_ctx->sx130x_cfg->state_timings.tx_delay_us < SX130X_FSK_PROCESS_DELAY_US_MIN
            || drv_ctx->sx130x_cfg->state_timings.tx_delay_us > SX130X_FSK_PROCESS_DELAY_US_MAX)
               ? SX130X_FSK_TX_PROCESS_DELAY_US
               : drv_ctx->sx130x_cfg->state_timings.tx_delay_us;
}

uint32_t sid_pal_radio_get_fsk_rx_process_delay(void)
{
    halo_drv_semtech_ctx_t *drv_ctx = sx130x_get_drv_ctx();
    return (drv_ctx->sx130x_cfg->state_timings.rx_delay_us < SX130X_FSK_PROCESS_DELAY_US_MIN
            || drv_ctx->sx130x_cfg->state_timings.rx_delay_us > SX130X_FSK_PROCESS_DELAY_US_MAX)
               ? SX130X_FSK_RX_PROCESS_DELAY_US
               : drv_ctx->sx130x_cfg->state_timings.rx_delay_us;
}
