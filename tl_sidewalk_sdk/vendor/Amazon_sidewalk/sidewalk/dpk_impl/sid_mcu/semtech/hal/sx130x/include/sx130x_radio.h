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

#ifndef SX130X_RADIO_H
#define SX130X_RADIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sx130x_halo.h>
#include <sx130x_config.h>
#include <sid_pal_serial_bus_ifc.h>
#include <sid_pal_radio_ifc.h>
#include <sid_pal_gpio_ifc.h>
#include <sid_time_types.h>

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

/*
 * @brief RADIO FSK status enumeration definition
 */
typedef enum {
    RADIO_FSK_RX_DONE_STATUS_OK                  = 0,
    RADIO_FSK_RX_DONE_STATUS_INVALID_PARAMETER   = 1,
    RADIO_FSK_RX_DONE_STATUS_INVALID_LENGTH      = 2,
    RADIO_FSK_RX_DONE_STATUS_BAD_CRC             = 3,
    RADIO_FSK_RX_DONE_STATUS_TIMEOUT             = 4,
    RADIO_FSK_RX_DONE_STATUS_UNKNOWN_ERROR       = 5,
    RADIO_FSK_RX_DONE_STATUS_SW_MARK_NOT_PRESENT = 6,
} radio_fsk_rx_done_status_t;

typedef struct {
    sid_pal_radio_modem_mode_t                   modem;
    sid_pal_radio_rx_packet_t                    *radio_rx_packet;
    sid_pal_radio_event_notify_t                 report_radio_event;

    uint8_t                                      radio_state;
    sid_pal_radio_irq_handler_t                  irq_handler;

    sid_pal_radio_cad_param_exit_mode_t          cad_exit_mode;
#if HALO_ENABLE_DIAGNOSTICS
    bool                                         pa_cfg_configured;
#endif

    uint32_t                                     irq_mask;
    uint32_t                                     radio_freq_hz;

    struct {
        sid_pal_radio_lora_packet_params_t       lora_pkt_params;
        sid_pal_radio_lora_modulation_params_t   lora_mod_params;
        sid_pal_radio_fsk_packet_params_t        fsk_pkt_params;
        sid_pal_radio_fsk_modulation_params_t    fsk_mod_params;
        sid_pal_radio_fsk_cad_params_t           fsk_cad_params;
    }                                            settings_cache;

    struct {
        uint8_t                                  stat1;
        uint8_t                                  stat2;
        uint16_t                                 command;
    }                                            last;

    //Append sx130x drv context
    const radio_sx130x_device_config_t           *sx130x_cfg;
    const struct sid_pal_serial_bus_iface        *sx130x_bus_iface;
    radio_sx130x_pa_cfg_t                        sx130x_pa_cfg;
    uint16_t                                     trim;
} halo_drv_semtech_ctx_t;

#define SX130X_US_TO_SYMBOLS(time_in_us, bit_rate) ((time_in_us*bit_rate) / SID_TIME_USEC_PER_SEC)

halo_drv_semtech_ctx_t* sx130x_get_drv_ctx(void);

void radio_sx130x_event_notify(sid_pal_radio_events_t radio_event);

int32_t radio_lora_process_rx_done(halo_drv_semtech_ctx_t *drv_ctx);

int32_t radio_fsk_process_sync_word_detected(halo_drv_semtech_ctx_t *drv_ctx);

int32_t radio_fsk_process_rx_done(halo_drv_semtech_ctx_t *drv_ctx,
                                  radio_fsk_rx_done_status_t *rx_done_status);

void set_lora_exit_mode(sid_pal_radio_cad_param_exit_mode_t cad_exit_mode);

int32_t set_radio_sx130x_trim_cap_val(uint16_t trim);

int32_t get_radio_sx130x_pa_config(radio_sx130x_pa_cfg_t *cfg);

#ifdef __cplusplus
}
#endif

#endif
