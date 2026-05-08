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

#include <sid_clock_ifc.h>
#include <sid_error.h>
#include <sid_pal_log_ifc.h>
#include <sid_pal_critical_region_ifc.h>
#include <sid_pal_timer_ifc.h>
#include <sid_time_ops.h>

#include <sx130x_halo.h>
#include <sx130x_hal.h>
#include <sx130x_radio.h>
#include <loragw_aux.h>
#include <loragw_reg.h>
#include <loragw_sx1302.h>
#include <loragw_test.h>

#include <string.h>

#define SX130X_RFCHAIN_FREQ_IN_HZ  910000000
#define SX130X_TIME_USEC_PER_MSEC  1000UL
#define SX130X_TX_MINIMUM_TIMEOUT  (5 * SX130X_TIME_USEC_PER_MSEC)
#define SX130X_RX_MINIMUM_TIMEOUT  (10 * SX130X_TIME_USEC_PER_MSEC)
#define SX130X_RX_DELAY_TIME  (0.5 * SX130X_TIME_USEC_PER_MSEC)
#define SX130X_RX_DROP_MS_TIME  1000
#define SX130X_TX_DEF_PWR_IN_DBM  14
#define SX130X_LORA_M_SF_IF_CHANNEL_NUM  8
#define SX130X_LGW_TEST  0
#define SX130X_RX_MAX_PKTS  16

#define SX130X_FDEV_IN_KHZ(X)  (X / 1000)

//SX130X GW re-setting paramters required to restart driver, and it will take about 3 Secs.
#ifndef SX130X_RESTART_ENABLE
#define SX130X_RESTART_ENABLE  0
#endif
static uint8_t sx130x_allow_drv_restart = SX130X_RESTART_ENABLE;

static struct lgw_conf_board_s boardconf;
static struct lgw_conf_rxrf_s rfconf;
static struct lgw_conf_rxif_s ifconf;
static struct lgw_conf_demod_s demodconf;
// TODO: Unknown issue prevents us to use malloc for txpkt/ rxpkt
static uint8_t rxpkt_buf[sizeof(struct lgw_pkt_rx_s) * SX130X_RX_MAX_PKTS] = {0};
static uint8_t txpkt_buf[sizeof(struct lgw_pkt_tx_s) * 1] = {0};
static struct lgw_pkt_tx_s *txpkt = (struct lgw_pkt_tx_s *)txpkt_buf;
static struct lgw_pkt_rx_s *rxpkt = (struct lgw_pkt_rx_s *)rxpkt_buf;

static sid_pal_timer_t sx130x_tx_timer;
static sid_pal_timer_t sx130x_tx_notify_timer;
static sid_pal_timer_t sx130x_rx_timer;
static uint32_t sx130x_rx_timeout = 0;
static bool sx130x_tx_timeout_flag = 0;

/**
 * @brief SX130X Radio Operation State
 */
enum sx130x_radio_state {
    SX130X_RADIO_OFF        = 0,
    SX130X_RADIO_TX         = ( 1 << 0 ),
    SX130X_RADIO_TX_DONE    = ( 1 << 1 ),
    SX130X_RADIO_TX_TIMEOUT = ( 1 << 2 ),
    SX130X_RADIO_RX         = ( 1 << 3 ),
    SX130X_RADIO_RX_DONE    = ( 1 << 4 ),
    SX130X_RADIO_RX_ERROR   = ( 1 << 5 ),
    SX130X_RADIO_RX_TIMEOUT = ( 1 << 6 ),
    SX130X_RADIO_CW         = ( 1 << 7 ),
    SX130X_RADIO_START      = ( 1 << 8 ),
    SX130X_RADIO_RESTART    = ( 1 << 9 ),
};

static enum sx130x_radio_state sx130x_radio_state = SX130X_RADIO_OFF;

static int32_t sx130x_set_radio_state(enum sx130x_radio_state states)
{
    sx130x_radio_state |= states;
    return RADIO_ERROR_NONE;
}

static void sx130x_clear_radio_state(enum sx130x_radio_state state)
{
    sx130x_radio_state &= ~state;
    return;
}

static void sx130x_radio_irq(void *arg)
{
    sid_pal_enter_critical_region();
    halo_drv_semtech_ctx_t *ctx = (halo_drv_semtech_ctx_t *)sx130x_get_drv_ctx();
    (void)arg;

    if (ctx) {
        sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &ctx->radio_rx_packet->rcv_tm, NULL);
        ctx->irq_handler();
    }
    sid_pal_exit_critical_region();
}

static void sx130x_tx_timer_cb(void *arg, sid_pal_timer_t *owner)
{
    if (&sx130x_tx_timer == owner) {
        sx130x_tx_timeout_flag = 1;
    } else if (&sx130x_tx_notify_timer == owner) {
        sx130x_radio_irq(NULL);
    }
    return;
}

static void sx130x_rx_timer_cb(void *arg, sid_pal_timer_t *owner)
{
    halo_drv_semtech_ctx_t *ctx = (halo_drv_semtech_ctx_t *)sx130x_get_drv_ctx();
    sid_pal_radio_rx_packet_t *pkt = (sid_pal_radio_rx_packet_t *)ctx->radio_rx_packet;
    sid_pal_radio_lora_rx_packet_status_t *pkt_status = &pkt->lora_rx_packet_status;

    int16_t nb_pkt = lgw_receive(1, rxpkt);
    if (nb_pkt > 0) {
        for (int i=0; i<nb_pkt; i++) {
            //Note: sx1302/ sx1303 driver will rx zero packet, we should drop it.
            if (rxpkt[i].size <= 0) {
                //drop packet
                break;
            } else if (rxpkt[i].size > SID_PAL_RADIO_RX_PAYLOAD_MAX_SIZE) {
                sx130x_set_radio_state(SX130X_RADIO_RX_ERROR);
                goto ret;
            }
            if (rxpkt[i].status != STAT_CRC_OK) {
                sx130x_set_radio_state(SX130X_RADIO_RX_ERROR);
                goto ret;
            }
            uint32_t tm_cnt;
            lgw_get_instcnt(&tm_cnt);
            tm_cnt = (tm_cnt - rxpkt[i].count_us) / SX130X_TIME_USEC_PER_MSEC;
            if (tm_cnt > SX130X_RX_DROP_MS_TIME) {
                //drop packet
                break;
            }

            sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &pkt->rcv_tm, NULL);
            pkt->payload_len = rxpkt[i].size;
            memcpy(pkt->rcv_payload, rxpkt[i].payload, rxpkt[i].size);

            pkt_status->rssi = (int8_t)rxpkt[i].rssic - ctx->sx130x_cfg->lna_gain;
            pkt_status->snr = (int8_t)rxpkt[i].snr;
            pkt_status->signal_rssi = (int8_t)rxpkt[i].rssis - ctx->sx130x_cfg->lna_gain;
            if (pkt_status->snr < 0) {
                pkt_status->rssi += pkt_status->snr;
            }
            sx130x_set_radio_state(SX130X_RADIO_RX_DONE);
            goto ret;
        }
    }
    if (sx130x_rx_timeout != 0) {
        struct sid_timespec cktime;
        sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &cktime, NULL);
        if ((sx130x_rx_timeout / SX130X_TIME_USEC_PER_MSEC) <= sid_timespec_to_ms(&cktime)) {
            if (!(sx130x_radio_state & SX130X_RADIO_RX_DONE)) {
                sx130x_set_radio_state(SX130X_RADIO_RX_TIMEOUT);
            }
        }
    }
ret:
    if ((sx130x_radio_state & SX130X_RADIO_RX_DONE) ||
        (sx130x_radio_state & SX130X_RADIO_RX_TIMEOUT) ||
        (sx130x_radio_state & SX130X_RADIO_RX_ERROR)) {
        sx130x_radio_irq(NULL);
    }
    return;
}

static int32_t sx130x_init_timers(void)
{
    int32_t err = RADIO_ERROR_INVALID_PARAMS;
    do {
        if (sid_pal_timer_init(&sx130x_tx_timer,
                               sx130x_tx_timer_cb, NULL) != SID_ERROR_NONE) {
            SID_HAL_LOG_ERROR("tx timer init failed");
            goto ret;
        }
        if (sid_pal_timer_init(&sx130x_tx_notify_timer,
                               sx130x_tx_timer_cb, NULL) != SID_ERROR_NONE) {
            SID_HAL_LOG_ERROR("tx notify timer init failed");
            goto ret;
        }
        if (sid_pal_timer_init(&sx130x_rx_timer,
                               sx130x_rx_timer_cb, NULL) != SID_ERROR_NONE) {
            SID_HAL_LOG_ERROR("rx timer init failed");
            goto ret;
        }
    } while (0);

    err = RADIO_ERROR_NONE;
ret:

    return err;
}

static void sx130x_deinit_timers(void)
{
    sid_pal_timer_deinit(&sx130x_tx_timer);
    sid_pal_timer_deinit(&sx130x_tx_notify_timer);
    sid_pal_timer_deinit(&sx130x_rx_timer);
}

int32_t sx130x_restart(void)
{
    int32_t err = RADIO_ERROR_INVALID_PARAMS;

    if (sx130x_radio_state & SX130X_RADIO_RESTART) {
        if (sx130x_radio_state & SX130X_RADIO_START) {
            halo_drv_semtech_ctx_t *ctx = (halo_drv_semtech_ctx_t *)sx130x_get_drv_ctx();
            sx130x_set_standby(ctx);
            sx130x_deinit_timers();
            if (sx130x_reset(ctx) != SX130X_STATUS_OK) {
                SID_HAL_LOG_ERROR("ERROR: failed to reset\n");
                goto ret;
            }
            lgw_stop();
            if (sx130x_init_timers() != RADIO_ERROR_NONE) {
                SID_HAL_LOG_ERROR("ERROR: failed to init timers\n");
                goto ret;
            }
            if (lgw_start() != LGW_HAL_SUCCESS) {
                SID_HAL_LOG_ERROR("ERROR: failed to lgw_start\n");
                sx130x_deinit_timers();
                goto ret;
            }
        }
        sx130x_clear_radio_state(SX130X_RADIO_RESTART);
    }

    err = RADIO_ERROR_NONE;
ret:

    return err;
}

int32_t sx130x_set_platform(void)
{
    int32_t err = RADIO_ERROR_INVALID_PARAMS;
    int i;
    halo_drv_semtech_ctx_t *ctx;

    if ( (ctx = (halo_drv_semtech_ctx_t *)sx130x_get_drv_ctx()) == NULL) {
        err = RADIO_ERROR_IO_ERROR;
        goto ret;
    }
    SID_HAL_LOG_INFO("%s ", lgw_version_info());

    if (SX130X_LGW_TEST) {
        lgw_test();
    }

    if (sx130x_reset(ctx) != SX130X_STATUS_OK) {
        err = RADIO_ERROR_IO_ERROR;
        goto ret;
    }

    /* Configure the gateway */
    memset(&boardconf, 0, sizeof(boardconf));
    boardconf.lorawan_public = false;
    boardconf.clksrc = 0;
    boardconf.full_duplex = false;
    boardconf.com_type = LGW_COM_SPI;
    if (lgw_board_setconf(&boardconf) != LGW_HAL_SUCCESS) {
        SID_HAL_LOG_ERROR("ERROR: failed to configure board\n");
        goto ret;
    }

    /* Set configuration for RF chain 0 - used for Tx */
    memset(&rfconf.enable, 0, sizeof(rfconf));
    rfconf.enable            = true;
    rfconf.freq_hz           = ctx->sx130x_cfg->channel_freq_hz?
                               ctx->sx130x_cfg->channel_freq_hz: SX130X_RFCHAIN_FREQ_IN_HZ;
    rfconf.type              = LGW_RADIO_TYPE_SX1250;
    rfconf.rssi_offset       = ctx->sx130x_cfg->rssi_offset[0];
    rfconf.tx_enable         = true;
    rfconf.single_input_mode = false;
    if (lgw_rxrf_setconf(0, &rfconf) != LGW_HAL_SUCCESS) {
        SID_HAL_LOG_ERROR("ERROR: failed to configure rxrf 0\n");
        goto ret;
    }

    /* Set configuration for RF chain 1 */
    memset(&rfconf.enable, 0, sizeof(rfconf));
    rfconf.enable            = true;
    rfconf.freq_hz           = ctx->sx130x_cfg->channel_freq_hz?
                               ctx->sx130x_cfg->channel_freq_hz: SX130X_RFCHAIN_FREQ_IN_HZ;
    rfconf.type              = LGW_RADIO_TYPE_SX1250;
    rfconf.rssi_offset       = ctx->sx130x_cfg->rssi_offset[1];
    rfconf.tx_enable         = false;
    rfconf.single_input_mode = false;
    if (lgw_rxrf_setconf(1, &rfconf) != LGW_HAL_SUCCESS) {
        SID_HAL_LOG_ERROR("ERROR: failed to configure rxrf 1\n");
        goto ret;
    }

    memset(&demodconf, 0, sizeof(demodconf));
    demodconf.multisf_datarate = LGW_MULTI_SF_EN;  // Set corresponding bit in the bitmask SF5 is LSB -> SF12 is MSB
    if (lgw_demod_setconf(&demodconf) != LGW_HAL_SUCCESS) {
        SID_HAL_LOG_ERROR( "ERROR: invalid configuration for demodulation parameters\n" );
        goto ret;
    }

    /* Set configuration for LoRa multi-SF channels (bandwidth set to 125kHz) */
    memset(&ifconf.enable, 0, sizeof(ifconf));
    for (i=0; i<SX130X_LORA_M_SF_IF_CHANNEL_NUM; i++) {
        ifconf.enable = true;
        ifconf.rf_chain = ctx->sx130x_cfg->channel_rfchain[i];
        ifconf.freq_hz = ctx->sx130x_cfg->channel_if[i];
        ifconf.datarate = DR_LORA_SF7;
        ifconf.implicit_crc_en = 1;
        ifconf.implicit_coderate = CR_LORA_4_5;
        if (lgw_rxif_setconf(i, &ifconf) != LGW_HAL_SUCCESS) {
            SID_HAL_LOG_ERROR("ERROR: failed to configure rxif %d\n", i);
            goto ret;
        }
    }

    /* Set configuration for LoRa Service channel */
    i = 8;
    memset(&ifconf.enable, 0, sizeof(ifconf));
    ifconf.enable = true;
    ifconf.rf_chain = ctx->sx130x_cfg->channel_rfchain[i];
    ifconf.freq_hz = ctx->sx130x_cfg->channel_if[i];
    ifconf.datarate = ctx->sx130x_cfg->high_speed_lora.sf?
                      ctx->sx130x_cfg->high_speed_lora.sf: DR_LORA_SF7;
    ifconf.bandwidth = ctx->sx130x_cfg->high_speed_lora.bw?
                       ctx->sx130x_cfg->high_speed_lora.bw: BW_250KHZ;
    ifconf.implicit_crc_en = ctx->sx130x_cfg->high_speed_lora.crc?
                             ctx->sx130x_cfg->high_speed_lora.crc: 0;
    ifconf.implicit_coderate = CR_LORA_4_5;
    if (lgw_rxif_setconf(8, &ifconf) != LGW_HAL_SUCCESS) {
        SID_HAL_LOG_ERROR("ERROR: failed to configure rxif for LoRa service channel\n");
        goto ret;
    }

    /* Set configuration for FSK Service channel */
#if 0// Not support FSK in sx1302/s1303 radio hal driver
    i = 9;
    memset(&ifconf.enable, 0, sizeof(ifconf));
    ifconf.enable = true;
    ifconf.rf_chain = ctx->sx130x_cfg->channel_rfchain[i];
    ifconf.freq_hz = ctx->sx130x_cfg->channel_if[i];
    ifconf.datarate = 50000;
    ifconf.bandwidth = BW_125KHZ;
    ifconf.sync_word_size = 3;
    ifconf.sync_word = 0x0055904e;
    if (lgw_rxif_setconf(9, &ifconf) != LGW_HAL_SUCCESS) {
        SID_HAL_LOG_ERROR("ERROR: failed to configure rxif for FSK service channel\n");
        goto ret;
    }
#endif

    /* Set the Tx configuration look-up table */
    if (lgw_txgain_setconf(0, (struct lgw_tx_gain_lut_s*)&ctx->sx130x_cfg->txlut) != LGW_HAL_SUCCESS) {
        SID_HAL_LOG_ERROR( "ERROR: failed to configure txgain lut\n");
        goto ret;
    }

    txpkt->rf_chain = 0;
    txpkt->tx_mode = IMMEDIATE;
    txpkt->freq_offset = 0;
    txpkt->f_dev = SX130X_FDEV_IN_KHZ(25000);
    txpkt->datarate = 50000;
    txpkt->freq_hz = SX130X_RFCHAIN_FREQ_IN_HZ;

    if (sx130x_init_timers() != RADIO_ERROR_NONE) {
        goto ret;
    }

    if (lgw_start() != LGW_HAL_SUCCESS) {
        SID_HAL_LOG_ERROR("ERROR: failed to lgw_start\n");
        goto ret;
    }
    sx130x_set_radio_state(SX130X_RADIO_START);

    err = RADIO_ERROR_NONE;
ret:
    if (err != RADIO_ERROR_NONE) {
        sx130x_deinit_timers();
    }

    return err;
}

void sx130x_radio_process(void)
{
    do {
        if (sx130x_radio_state & SX130X_RADIO_TX) {
            if (sx130x_radio_state & SX130X_RADIO_TX_DONE) {
                sx130x_clear_radio_state(SX130X_RADIO_TX_DONE);
                radio_sx130x_event_notify(SID_PAL_RADIO_EVENT_TX_DONE);
                break;
            }
            if (sx130x_radio_state & SX130X_RADIO_TX_TIMEOUT) {
                sx130x_clear_radio_state(SX130X_RADIO_TX_TIMEOUT);
                radio_sx130x_event_notify(SID_PAL_RADIO_EVENT_TX_TIMEOUT);
                break;
            }
        }
        if (sx130x_radio_state & SX130X_RADIO_RX) {
            if (sx130x_radio_state & SX130X_RADIO_RX_DONE) {
                sx130x_clear_radio_state(SX130X_RADIO_RX_DONE);
                radio_sx130x_event_notify(SID_PAL_RADIO_EVENT_RX_DONE);
                break;
            }
            if (sx130x_radio_state & SX130X_RADIO_RX_TIMEOUT) {
                sx130x_clear_radio_state(SX130X_RADIO_RX_TIMEOUT);
                radio_sx130x_event_notify(SID_PAL_RADIO_EVENT_RX_TIMEOUT);
                break;
            }
            if (sx130x_radio_state & SX130X_RADIO_RX_ERROR) {
                sx130x_clear_radio_state(SX130X_RADIO_RX_ERROR);
                radio_sx130x_event_notify(SID_PAL_RADIO_EVENT_RX_ERROR);
                break;
            }
        }
    } while (0);
}

int32_t sx130x_set_txpower(int8_t pwr)
{
    txpkt->rf_power = pwr;

    return RADIO_ERROR_NONE;
}

int32_t sx130x_set_rf_freq(const uint32_t freq_in_hz)
{
    lgw_context_t *ctx = (lgw_context_t *)lgw_get_context();

    txpkt->freq_hz = freq_in_hz;

    if (sx130x_allow_drv_restart) {
        bool update_channel = true;

        for (int i=0; i<LGW_RF_CHAIN_NB; i++) {
            if (freq_in_hz != ctx->rf_chain_cfg[i].freq_hz) {
                uint32_t if_freq = 0;
                for (int j=0; j<(SX130X_LORA_M_SF_IF_CHANNEL_NUM + 1); j++) {
                    if_freq = ctx->if_chain_cfg[j].freq_hz;
                    if (freq_in_hz == ctx->rf_chain_cfg[i].freq_hz + if_freq) {
                        update_channel = false;
                        break;
                    }
                }
                if (update_channel == false) {
                    break;
                }
            } else {
                update_channel = false;
                break;
            }
        }
        //update chain0 & chain1 freq.
        if (update_channel) {
            for (int i=0; i<LGW_RF_CHAIN_NB; i++) {
                if (ctx->rf_chain_cfg[i].freq_hz != freq_in_hz) {
                    ctx->rf_chain_cfg[i].freq_hz = freq_in_hz;
                    sx130x_set_radio_state(SX130X_RADIO_RESTART);
                }
            }
        }
    }

    return RADIO_ERROR_NONE;
}

int32_t sx130x_set_tx_payload(const uint8_t *buffer, uint8_t size)
{
    txpkt->size = size;
    memcpy(txpkt->payload, buffer, txpkt->size);

    return RADIO_ERROR_NONE;
}

static void sx130x_set_tx_notify(void)
{
    struct sid_timespec first_shot, tm;
    sid_ms_to_timespec(0, &tm);
    sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &first_shot, NULL);
    sid_time_add(&first_shot, &tm);
    sid_pal_timer_arm(&sx130x_tx_notify_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE,
                      &first_shot, NULL);
}

int32_t sx130x_set_tx(const void *context, const uint32_t timeout)
{
    int32_t err = RADIO_ERROR_IO_ERROR;
    struct sid_timespec first_shot, tm;
    halo_drv_semtech_ctx_t *ctx = (halo_drv_semtech_ctx_t *)context;
    uint32_t tx_timeout = (timeout <= SX130X_TX_MINIMUM_TIMEOUT)?
                          SX130X_TX_MINIMUM_TIMEOUT: timeout;

    txpkt->modulation = (ctx->modem == SID_PAL_RADIO_MODEM_MODE_FSK)? MOD_FSK: MOD_LORA;
    txpkt->tx_mode = IMMEDIATE;
    txpkt->rf_chain = 0;

    sid_ms_to_timespec(tx_timeout / SX130X_TIME_USEC_PER_MSEC, &tm);
    sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &first_shot, NULL);
    sid_time_add(&first_shot, &tm);

    if (lgw_send(txpkt) != 0) {
        SID_HAL_LOG_ERROR("ERROR: failed to send packet\n");
        goto ret;
    }
    sx130x_set_radio_state(SX130X_RADIO_TX);

    sx130x_tx_timeout_flag = 0;
    if (sid_pal_timer_arm(&sx130x_tx_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE,
                          &first_shot, NULL) != SID_ERROR_NONE) {
        goto ret;
    }

    uint8_t tx_status;
    do {
        wait_ms(10);
        lgw_status(txpkt->rf_chain, TX_STATUS, &tx_status);
        if (sx130x_tx_timeout_flag) {
            break;
        }
    } while (tx_status != TX_FREE);
    if (sx130x_tx_timeout_flag) {
        sx130x_set_radio_state(SX130X_RADIO_TX_TIMEOUT);
    } else {
        sx130x_set_radio_state(SX130X_RADIO_TX_DONE);
    }
    sx130x_set_tx_notify();
    err = RADIO_ERROR_NONE;
ret:
    sid_pal_timer_cancel(&sx130x_tx_timer);

    return err;
}

int32_t sx130x_set_tx_cw(const void *context)
{
    int32_t err = RADIO_ERROR_NONE;

    txpkt->rf_chain = 0;
    txpkt->modulation = MOD_CW;
    txpkt->tx_mode = IMMEDIATE;
    txpkt->invert_pol = 0;
    txpkt->preamble = 0;
    txpkt->no_header = 0;
    txpkt->freq_offset = 0;
    txpkt->f_dev = SX130X_FDEV_IN_KHZ(25000);
    txpkt->size = 10;
    for (int i=0; i<txpkt->size; i++) {
        txpkt->payload[i] = (uint8_t)RAND_RANGE(0, 255);
    }
    if (lgw_send(txpkt) != 0) {
        SID_HAL_LOG_ERROR("ERROR: failed to send packet\n");
        err = RADIO_ERROR_IO_ERROR;
        goto ret;
    }
    sx130x_set_radio_state(SX130X_RADIO_CW);
ret:

    return err;
}

int32_t sx130x_set_rx(const void *context, const uint32_t timeout)
{
    int32_t err = RADIO_ERROR_IO_ERROR;
    struct sid_timespec first_shot;
    struct sid_timespec period = { .tv_sec = 0, .tv_nsec = 10000000 };//wait 10ms for 16 pkt

    sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &first_shot, NULL);
    first_shot.tv_nsec += SX130X_RX_DELAY_TIME;
    sx130x_rx_timeout = (timeout == 0xFFFFFF)? 0: (timeout <= SX130X_RX_MINIMUM_TIMEOUT)?
                        SX130X_RX_MINIMUM_TIMEOUT: timeout;
    if (sid_pal_timer_arm(&sx130x_rx_timer, SID_PAL_TIMER_PRIO_CLASS_PRECISE,
                          &first_shot, &period) != SID_ERROR_NONE) {
        goto ret;
    }
    sx130x_set_radio_state(SX130X_RADIO_RX);

    err = RADIO_ERROR_NONE;
ret:

    if (err != RADIO_ERROR_NONE) {
        sid_pal_timer_cancel(&sx130x_rx_timer);
    }

    return err;
}

int32_t sx130x_set_standby(const void *context)
{
    if (sx130x_radio_state & SX130X_RADIO_TX) {
        if (!(sx130x_radio_state & SX130X_RADIO_TX_DONE)) {
            lgw_abort_tx(0);
        }
        sx130x_clear_radio_state(SX130X_RADIO_TX);
        sx130x_clear_radio_state(SX130X_RADIO_TX_DONE);
    }
    if (sx130x_radio_state & SX130X_RADIO_RX) {
        sid_pal_timer_cancel(&sx130x_rx_timer);
        sx130x_clear_radio_state(SX130X_RADIO_RX);
        sx130x_clear_radio_state(SX130X_RADIO_RX_DONE);
    }
    if (sx130x_radio_state & SX130X_RADIO_CW) {
        lgw_abort_tx(0);
        sx130x_clear_radio_state(SX130X_RADIO_CW);
    }
    sx130x_rx_timeout = 0;

    return RADIO_ERROR_NONE;
}

int32_t sx130x_reset(const void *context)
{
    return (int32_t)sx130x_hal_reset(context);
}

int32_t sx130x_set_lora_sync_word(const void *context, const uint8_t sync_word)
{
    lgw_context_t *ctx = (lgw_context_t *)lgw_get_context();
    int32_t err = RADIO_ERROR_IO_ERROR;

    if (sync_word == 0x12) { //PRIVATE
        if (sx1302_lora_syncword(false, 0) != LGW_REG_SUCCESS) {
            goto ret;
        }
    } else if (sync_word == 0x34) { //PUBLIC
        if (sx1302_lora_syncword(true, 0) != LGW_REG_SUCCESS) {
            goto ret;
        }
    } else {
        if (sx1302_lora_syncword(false, 0) != LGW_REG_SUCCESS) {
            goto ret;
        }
    }
    if (sx130x_allow_drv_restart) {
        if (ctx->board_cfg.lorawan_public != (sync_word == 0x12)? false: true) {
            ctx->board_cfg.lorawan_public = (sync_word == 0x12)? false: true;
            sx130x_set_radio_state(SX130X_RADIO_RESTART);
        }
    }
    err = RADIO_ERROR_NONE;
ret:

    return err;
}

int32_t sx130x_set_lora_mod_params(const void *context, const sx130x_mod_params_lora_t *params)
{
    int32_t err = RADIO_ERROR_INVALID_PARAMS;
    lgw_context_t *ctx = (lgw_context_t *)lgw_get_context();

    if ((params->sf >= SX130X_LORA_SF5) &&
        (params->sf <= SX130X_LORA_SF12)) {
        txpkt->datarate = params->sf;
    } else {
        goto ret;
    }
    if (params->bw <= SX130X_LORA_BW_125) {
        txpkt->bandwidth = BW_125KHZ;
    } else if (params->bw <= SX130X_LORA_BW_250) {
        txpkt->bandwidth = BW_250KHZ;
    } else if (params->bw <= SX130X_LORA_BW_500) {
        txpkt->bandwidth = BW_500KHZ;
    } else {
        goto ret;
    }
    if (params->cr <= SX130X_LORA_CR_4_5) {
        txpkt->coderate = CR_LORA_4_5;
    } else if (params->cr <= SX130X_LORA_CR_4_6) {
        txpkt->coderate = CR_LORA_4_6;
    } else if (params->cr <= SX130X_LORA_CR_4_7) {
        txpkt->coderate = CR_LORA_4_7;
    } else if (params->cr <= SX130X_LORA_CR_4_8) {
        txpkt->coderate = CR_LORA_4_8;
    } else {
        goto ret;
    }

    if (sx130x_allow_drv_restart) {
        if ((ctx->lora_service_cfg.bandwidth != txpkt->bandwidth) ||
            (ctx->lora_service_cfg.datarate != txpkt->datarate) ||
            (ctx->lora_service_cfg.implicit_coderate != txpkt->coderate)) {
            ctx->lora_service_cfg.bandwidth = txpkt->bandwidth;
            ctx->lora_service_cfg.datarate = txpkt->datarate;
            ctx->lora_service_cfg.implicit_coderate = txpkt->coderate;
            sx130x_set_radio_state(SX130X_RADIO_RESTART);
        }
    }
    err = RADIO_ERROR_NONE;
ret:

    return err;
}

int32_t sx130x_set_lora_pkt_params(const void *context, const sx130x_pkt_params_lora_t *params)
{
    lgw_context_t *ctx = (lgw_context_t *)lgw_get_context();

    txpkt->preamble = params->pbl_len_in_symb;
    txpkt->no_header = params->hdr_type; /*0:EXPLICIT, 1:IMPLICIT*/
    txpkt->no_crc = params->crc_is_on? 0: 1;
    txpkt->invert_pol = params->invert_iq_is_on? 1: 0;

    if (sx130x_allow_drv_restart) {
        if ((ctx->lora_service_cfg.implicit_hdr != txpkt->no_header) ||
            (ctx->lora_service_cfg.implicit_crc_en = txpkt->no_crc)) {
            ctx->lora_service_cfg.implicit_hdr = txpkt->no_header;
            ctx->lora_service_cfg.implicit_crc_en = txpkt->no_crc;
            sx130x_set_radio_state(SX130X_RADIO_RESTART);
        }
    }

    return RADIO_ERROR_NONE;
}

int32_t sx130x_set_cad_params(const void *context, const sx130x_lora_cad_params_t *params)
{
    return RADIO_ERROR_NONE;
}

int32_t sx130x_get_rssi_inst(const void *context, int16_t *rssi_in_dbm)
{
    *rssi_in_dbm = 0;
    return RADIO_ERROR_NONE;
}

uint32_t sx130x_get_lora_bw_in_hz(sx130x_lora_bw_t bw)
{
    uint32_t bw_in_hz = 0;

    switch( bw )
    {
    case SX130X_LORA_BW_007:
        bw_in_hz = 7812UL;
        break;
    case SX130X_LORA_BW_010:
        bw_in_hz = 10417UL;
        break;
    case SX130X_LORA_BW_015:
        bw_in_hz = 15625UL;
        break;
    case SX130X_LORA_BW_020:
        bw_in_hz = 20833UL;
        break;
    case SX130X_LORA_BW_031:
        bw_in_hz = 31250UL;
        break;
    case SX130X_LORA_BW_041:
        bw_in_hz = 41667UL;
        break;
    case SX130X_LORA_BW_062:
        bw_in_hz = 62500UL;
        break;
    case SX130X_LORA_BW_125:
        bw_in_hz = 125000UL;
        break;
    case SX130X_LORA_BW_250:
        bw_in_hz = 250000UL;
        break;
    case SX130X_LORA_BW_500:
        bw_in_hz = 500000UL;
        break;
    }

    return bw_in_hz;
}

uint32_t sx130x_get_lora_time_on_air_numerator(const sx130x_pkt_params_lora_t *pkt_p,
                                               const sx130x_mod_params_lora_t *mod_p)
{
    const int32_t pld_len_in_bytes = pkt_p->pld_len_in_bytes;
    const int32_t sf = mod_p->sf;
    const bool pld_is_fix = pkt_p->hdr_type == SX130X_LORA_PKT_IMPLICIT;

    int32_t fine_synch = (sf <= 6)? 1: 0;
    bool long_interleaving = (mod_p->cr > 4);

    int32_t total_bytes_nb = pld_len_in_bytes + (pkt_p->crc_is_on ?2 :0);
    int32_t tx_bits_symbol = sf - 2 * (mod_p->ldro != 0? 1: 0);

    int32_t ceil_numerator;
    int32_t ceil_denominator;

    int32_t intermed;

    int32_t symbols_nb_data;
    int32_t  tx_infobits_header;
    int32_t  tx_infobits_payload;

    if (long_interleaving) {
        const int32_t fec_rate_numerator = 4;
        const int32_t fec_rate_denominator = ( mod_p->cr + ( mod_p->cr == 7 ? 1 : 0 ) );

        if (pld_is_fix) {
            int32_t tx_bits_symbol_start = sf - 2 + 2 * fine_synch;
            if (8 * total_bytes_nb * fec_rate_denominator <=
                7 * fec_rate_numerator * tx_bits_symbol_start) {
                ceil_numerator = 8 * total_bytes_nb * fec_rate_denominator;
                ceil_denominator = fec_rate_numerator * tx_bits_symbol_start;
            } else {
                int32_t tx_codedbits_header = tx_bits_symbol_start * 8;
                ceil_numerator = 8 * fec_rate_numerator * tx_bits_symbol +
                                 8 * total_bytes_nb * fec_rate_denominator -
                                 fec_rate_numerator * tx_codedbits_header;
                ceil_denominator = fec_rate_numerator * tx_bits_symbol;
            }
        } else {
            tx_infobits_header = (sf * 4 + fine_synch * 8 - 28) & ~0x07;
            if (tx_infobits_header < 8 * total_bytes_nb) {
                if (tx_infobits_header > 8 * pld_len_in_bytes) {
                    tx_infobits_header = 8 * pld_len_in_bytes;
                }
            }
            tx_infobits_payload = 8 * total_bytes_nb - tx_infobits_header;
            if (tx_infobits_payload < 0) {
                tx_infobits_payload = 0;
            }

            ceil_numerator = tx_infobits_payload * fec_rate_denominator +
                             8 * fec_rate_numerator * tx_bits_symbol;
            ceil_denominator = fec_rate_numerator * tx_bits_symbol;
        }
    } else {
        tx_infobits_header = sf * 4 + fine_synch * 8 - 8;

        if (!pld_is_fix) {
            tx_infobits_header -= 20;
        }
        tx_infobits_payload = 8 * total_bytes_nb - tx_infobits_header;

        if (tx_infobits_payload < 0) {
            tx_infobits_payload = 0;
        }
        ceil_numerator   = tx_infobits_payload;
        ceil_denominator = 4 * tx_bits_symbol;
    }

    symbols_nb_data = ((ceil_numerator + ceil_denominator - 1) / ceil_denominator);
    if (!long_interleaving) {
        symbols_nb_data = symbols_nb_data * ( mod_p->cr + 4 ) + 8;
    }
    intermed = pkt_p->pbl_len_in_symb + 4 + 2 * fine_synch + symbols_nb_data;

    return (uint32_t )((4 * intermed + 1) * (1 << (sf - 2))) - 1;
}

uint32_t sx130x_get_lora_cad_duration_microsecs(uint8_t symbol ,
                                                const sx130x_mod_params_lora_t *mod_p)
{
    return 2000;
}

uint32_t sx130x_get_lora_time_on_air_in_ms(const sx130x_pkt_params_lora_t *pkt_p,
                                           const sx130x_mod_params_lora_t *mod_p)
{
    uint32_t numerator = 1000U * sx130x_get_lora_time_on_air_numerator(pkt_p, mod_p);
    uint32_t denominator = sx130x_get_lora_bw_in_hz(mod_p->bw);
    // Perform integral ceil()
    return (numerator + denominator - 1) / denominator;
}
