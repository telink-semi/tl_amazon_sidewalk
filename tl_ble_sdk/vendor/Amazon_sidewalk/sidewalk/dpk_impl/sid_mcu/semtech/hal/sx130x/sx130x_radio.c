/*
 * Copyright 2021-2025 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <sid_pal_assert_ifc.h>
#include <sid_pal_delay_ifc.h>
#include <sid_pal_log_ifc.h>
#include <sid_time_ops.h>

#include <sx130x_halo.h>
#include <sx130x_radio.h>

#define RX_CONTINUOUS_VAL                  0xFFFFFF
#define INFINITE_TIME                      0xFFFFFFFF

#define SX130X_MIN_CHANNEL_FREE_DELAY_US   1
#define SX130X_MIN_CHANNEL_NOISE_DELAY_US  30
#define SX130X_NOISE_SAMPLE_SIZE           32
#define SX130X_RF_RANDOM_TIMES             8

// Delay time to allow for any external PA/FEM turn ON/OFF
#define SEMTECH_STDBY_STATE_DELAY_US       10

#define LR1110_CAD_DEFAULT_TX_TIMEOUT      0 // disable Tx timeout for CAD
#define US_IN_SEC                          1000000UL

static halo_drv_semtech_ctx_t              drv_ctx = {0};

static int32_t radio_sx130x_platform_init(void)
{
    int32_t err = RADIO_ERROR_INVALID_PARAMS;

    if (drv_ctx.sx130x_cfg->gpios.tx_bypass != HALO_GPIO_NOT_CONNECTED) {
        if (sid_pal_gpio_set_direction(drv_ctx.sx130x_cfg->gpios.tx_bypass,
            SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            goto ret;
        }
    }

    if (drv_ctx.sx130x_cfg->gpios.rf_sw_ena != HALO_GPIO_NOT_CONNECTED) {
        if (sid_pal_gpio_set_direction(drv_ctx.sx130x_cfg->gpios.rf_sw_ena,
            SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            goto ret;
        }
    }

    if (drv_ctx.sx130x_cfg->gpios.txrx != HALO_GPIO_NOT_CONNECTED) {
        if (sid_pal_gpio_set_direction(drv_ctx.sx130x_cfg->gpios.txrx,
            SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            goto ret;
        }
    }

    if (drv_ctx.sx130x_cfg->gpios.power != HALO_GPIO_NOT_CONNECTED) {
        if (sid_pal_gpio_set_direction(drv_ctx.sx130x_cfg->gpios.power,
            SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            goto ret;
        }
    }

    if (drv_ctx.sx130x_cfg->pa_cfg_callback == NULL) {
        goto ret;
    }

    if (sx130x_set_platform() != RADIO_ERROR_NONE) {
        goto ret;
    }
    err = RADIO_ERROR_NONE;

ret:
    return err;
}

static int32_t sid_pal_radio_set_modem_to_lora_mode(void)
{
    drv_ctx.modem = SID_PAL_RADIO_MODEM_MODE_LORA;
    return RADIO_ERROR_NONE;
}

static int32_t sid_pal_radio_set_modem_to_fsk_mode(void)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}

halo_drv_semtech_ctx_t *sx130x_get_drv_ctx(void)
{
    return &drv_ctx;
}

void radio_sx130x_event_notify(sid_pal_radio_events_t radio_event)
{
    if (drv_ctx.report_radio_event) {
        drv_ctx.report_radio_event(radio_event);
    }

    return;
}

void set_lora_exit_mode(sid_pal_radio_cad_param_exit_mode_t cad_exit_mode)
{
    drv_ctx.cad_exit_mode = cad_exit_mode;
}

static int32_t radio_sx130x_set_radio_mode(bool rf_en, bool tx_en)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if (rf_en == false && tx_en == true) {
            /* Invalid pin config */
            err = RADIO_ERROR_INVALID_PARAMS;
            break;
        }

        if (drv_ctx.sx130x_cfg->gpios.rf_sw_ena != HALO_GPIO_NOT_CONNECTED) {
            if (sid_pal_gpio_write(drv_ctx.sx130x_cfg->gpios.rf_sw_ena,
                                   rf_en) != SID_ERROR_NONE) {
                err = RADIO_ERROR_IO_ERROR;
                break;
            }
        }

        if (drv_ctx.sx130x_cfg->gpios.txrx != HALO_GPIO_NOT_CONNECTED) {
            if (sid_pal_gpio_write(drv_ctx.sx130x_cfg->gpios.txrx,
                                   tx_en) != SID_ERROR_NONE) {
                err = RADIO_ERROR_IO_ERROR;
                break;
            }
        }

        if (drv_ctx.sx130x_cfg->gpios.tx_bypass != HALO_GPIO_NOT_CONNECTED) {
            if (sid_pal_gpio_write(drv_ctx.sx130x_cfg->gpios.tx_bypass,
                                   0 /*(tx_en && drv_ctx.ext_pa)*/) != SID_ERROR_NONE) {
                err = RADIO_ERROR_IO_ERROR;
                break;
            }
        }

        if (rf_en == true && tx_en == false) {
            if (sx130x_restart() != RADIO_ERROR_NONE) {
                err = RADIO_ERROR_IO_ERROR;
                break;
            }
        }
    } while (0);

    return err;
}

void set_radio_sx130x_device_config(const radio_sx130x_device_config_t *cfg)
{
    drv_ctx.sx130x_cfg = cfg;
}

uint8_t sid_pal_radio_get_status(void)
{
    return drv_ctx.radio_state;
}

sid_pal_radio_modem_mode_t sid_pal_radio_get_modem_mode(void)
{
    return drv_ctx.modem;
}

int32_t sid_pal_radio_set_modem_mode(sid_pal_radio_modem_mode_t mode)
{
    if (mode == SID_PAL_RADIO_MODEM_MODE_LORA) {
        return sid_pal_radio_set_modem_to_lora_mode();
    } else if (mode == SID_PAL_RADIO_MODEM_MODE_FSK) {
        return sid_pal_radio_set_modem_to_fsk_mode();
    }
    return RADIO_ERROR_NOT_SUPPORTED;
}

int32_t sid_pal_radio_irq_process(void)
{
    sx130x_radio_process();
    return RADIO_ERROR_NONE;
}

int32_t set_radio_sx130x_trim_cap_val(uint16_t trim)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}

uint16_t get_radio_sx130x_trim_cap_val(void)
{
    return drv_ctx.trim;
}

static int32_t sid_pal_sx130x_set_frequency(uint32_t freq)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if (sx130x_set_rf_freq(freq) != RADIO_ERROR_NONE) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }
    } while (0);

    return err;
}

int32_t sid_pal_radio_set_frequency(uint32_t freq)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if (drv_ctx.radio_state != SID_PAL_RADIO_STANDBY) {
            err = RADIO_ERROR_INVALID_STATE;
            break;
        }

        if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_FSK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        } else if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_LORA) {
            if (sid_pal_sx130x_set_frequency(freq) != RADIO_ERROR_NONE) {
                err = RADIO_ERROR_HARDWARE_ERROR;
                break;
            }
        }

        drv_ctx.radio_freq_hz = freq;
    } while (0);

    return err;
}

int32_t sid_pal_radio_get_max_tx_power(sid_pal_radio_data_rate_t data_rate, int8_t *tx_power)
{
    if (data_rate <= SID_PAL_RADIO_DATA_RATE_INVALID ||
        data_rate > SID_PAL_RADIO_DATA_RATE_MAX_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *tx_power = drv_ctx.sx130x_cfg->max_tx_power[data_rate - 1];

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_region(sid_pal_radio_region_code_t region)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}

#if HALO_ENABLE_DIAGNOSTICS
int32_t semtech_radio_set_sx126x_pa_config(semtech_radio_pa_cfg_t *cfg)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}
#endif

int32_t get_radio_sx130x_pa_config(radio_sx130x_pa_cfg_t *cfg)
{
    if (!cfg) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *cfg = drv_ctx.sx130x_pa_cfg;

    return RADIO_ERROR_NONE;
}

static int32_t sid_pal_sx130x_set_tx_power(int8_t power)
{
    if (sx130x_set_txpower(power) != RADIO_ERROR_NONE) {
        return RADIO_ERROR_IO_ERROR;
    }
    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_tx_power(int8_t power)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if (drv_ctx.radio_state != SID_PAL_RADIO_STANDBY) {
            err =  RADIO_ERROR_INVALID_STATE;
            break;
        }

        if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_FSK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        } else if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_LORA) {
            if (sid_pal_sx130x_set_tx_power(power) != RADIO_ERROR_NONE) {
                err = RADIO_ERROR_HARDWARE_ERROR;
                break;
            }
        }

    } while (0);

    return err;
}

int32_t sid_pal_radio_sleep(uint32_t sleep_us)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if (drv_ctx.radio_state == SID_PAL_RADIO_SLEEP) {
           goto ret;
        }

        if ((err = radio_sx130x_set_radio_mode(false, false)) != RADIO_ERROR_NONE) {
            goto ret;
        }
        drv_ctx.radio_state = SID_PAL_RADIO_SLEEP;

    } while (0);

ret:
    return err;
}

static int32_t sid_pal_sx130x_set_standby(void)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if ((err = radio_sx130x_set_radio_mode(false, false)) != RADIO_ERROR_NONE) {
            break;
        }

        sid_pal_delay_us(SEMTECH_STDBY_STATE_DELAY_US);

        if (sx130x_set_standby(&drv_ctx) != RADIO_ERROR_NONE) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }
    } while (0);

    return err;
}

int32_t sid_pal_radio_standby(void)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if (drv_ctx.radio_state == SID_PAL_RADIO_STANDBY) {
            break;
        }

        if (sid_pal_sx130x_set_standby() != RADIO_ERROR_NONE) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }
        drv_ctx.radio_state = SID_PAL_RADIO_STANDBY;

    } while (0);

    return err;
}

static int32_t sid_pal_sx130x_set_tx_payload(const uint8_t *buffer, uint8_t size)
{
    if (buffer == NULL || size == 0) {
        return RADIO_ERROR_INVALID_PARAMS;
    }

    if (sx130x_set_tx_payload(buffer, size) != RADIO_ERROR_NONE) {
        return RADIO_ERROR_IO_ERROR;
    }
    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_tx_payload(const uint8_t *buffer, uint8_t size)
{
    int32_t err = RADIO_ERROR_NONE;

    if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_FSK) {
        err = RADIO_ERROR_IO_ERROR;
        goto ret;
    } else if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_LORA) {
        if (sid_pal_sx130x_set_tx_payload(buffer, size) != RADIO_ERROR_NONE) {
            err = RADIO_ERROR_IO_ERROR;
            goto ret;
        }
    }

ret:
    return err;
}

static int32_t sid_pal_sx130x_radio_start_tx(uint32_t timeout)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if ((err = radio_sx130x_set_radio_mode(true, true)) != RADIO_ERROR_NONE) {
            break;
        }

        if (sx130x_set_tx(&drv_ctx, timeout) != SX130X_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }
    } while (0);

    return err;
}

int32_t sid_pal_radio_start_tx(uint32_t timeout)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_FSK) {
            err = RADIO_ERROR_IO_ERROR;
            goto ret;
        } else if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_LORA) {
            if ((err = sid_pal_sx130x_radio_start_tx(timeout)) != RADIO_ERROR_NONE) {
                goto ret;
            }
        }
        drv_ctx.radio_state = SID_PAL_RADIO_TX;

     } while (0);

ret:
    return err;
}

static int32_t sid_pal_sx130x_set_tx_cw(void)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if (sx130x_set_standby(&drv_ctx) != RADIO_ERROR_NONE) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }

        if ((err = radio_sx130x_set_radio_mode(true, true)) != RADIO_ERROR_NONE) {
            break;
        }

        if (sx130x_set_tx_cw(&drv_ctx) != SX130X_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }
    } while (0);

    return err;
}

int32_t sid_pal_radio_set_tx_continuous_wave(uint32_t freq, int8_t power)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if ((err = sid_pal_radio_set_frequency(freq) != RADIO_ERROR_NONE)) {
            goto ret;
        }

        if ((err = sid_pal_radio_set_tx_power(power) != RADIO_ERROR_NONE)) {
            goto ret;
        }

        if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_FSK) {
            err = RADIO_ERROR_IO_ERROR;
            goto ret;
        } else if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_LORA) {
            if (sid_pal_sx130x_set_tx_cw() != RADIO_ERROR_NONE) {
                err = RADIO_ERROR_IO_ERROR;
                goto ret;
            }
        }
        drv_ctx.radio_state = SID_PAL_RADIO_TX;
    } while (0);

ret:
    return err;
}

int32_t sid_pal_radio_set_tx_continuous_preamble(uint32_t freq, int8_t power)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}

static int32_t sid_pal_sx130x_start_rx(uint32_t timeout)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if ((err = radio_sx130x_set_radio_mode(true, false)) != RADIO_ERROR_NONE) {
            break;
        }

        if (sx130x_set_rx(&drv_ctx, timeout) != SX130X_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }
    } while (0);

    return err;
}

int32_t sid_pal_radio_start_rx(uint32_t timeout)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_FSK) {
            err = RADIO_ERROR_IO_ERROR;
            goto ret;
        } else if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_LORA) {
            if (sid_pal_sx130x_start_rx(timeout) != RADIO_ERROR_NONE) {
                err = RADIO_ERROR_IO_ERROR;
                goto ret;
            }
        }
        drv_ctx.radio_state = SID_PAL_RADIO_RX;
     } while (0);

ret:
    return err;
}

int32_t sid_pal_radio_is_cad_exit_mode(sid_pal_radio_cad_param_exit_mode_t exit_mode)
{
    int32_t err = RADIO_ERROR_NONE;

    if (!((exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_CS_ONLY) ||
        (exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_CS_RX) ||
        (exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_CS_LBT) ||
        (exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_ED_ONLY) ||
        (exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_ED_RX) ||
        (exit_mode == SID_PAL_RADIO_CAD_EXIT_MODE_ED_LBT))) {
        err = RADIO_ERROR_INVALID_PARAMS;
    }

    return err;
}

int32_t sid_pal_radio_start_carrier_sense(const sid_pal_radio_fsk_cad_params_t *cad_params,
                                          sid_pal_radio_cad_param_exit_mode_t exit_mode)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if (drv_ctx.modem != SID_PAL_RADIO_MODEM_MODE_FSK) {
            err = RADIO_ERROR_INVALID_PARAMS;
            break;
        }

        if ((err = sid_pal_radio_is_cad_exit_mode(exit_mode)) != RADIO_ERROR_NONE) {
            break;
        }

        drv_ctx.settings_cache.fsk_cad_params = *cad_params;
        drv_ctx.radio_state = SID_PAL_RADIO_RX;
        drv_ctx.cad_exit_mode = exit_mode;
     } while (0);

    return err;
}

static int32_t sid_pal_sx130x_start_continuous_rx(void)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if ((err = radio_sx130x_set_radio_mode(true, false)) != RADIO_ERROR_NONE) {
            break;
        }

        if (sx130x_set_rx(&drv_ctx, RX_CONTINUOUS_VAL) != SX130X_STATUS_OK) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            break;
        }
    } while (0);

    return err;
}

int32_t sid_pal_radio_start_continuous_rx(void)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_FSK) {
            err = RADIO_ERROR_IO_ERROR;
            goto ret;
        } else if (drv_ctx.modem == SID_PAL_RADIO_MODEM_MODE_LORA) {
            if (sid_pal_sx130x_start_continuous_rx() != RADIO_ERROR_NONE) {
                err = RADIO_ERROR_IO_ERROR;
                goto ret;
            }
        }
        drv_ctx.radio_state = SID_PAL_RADIO_RX;
    } while (0);

ret:
    return err;
}

int32_t sid_pal_radio_set_rx_duty_cycle(uint32_t rx_time, uint32_t sleep_time)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}

int32_t sid_pal_radio_lora_start_cad(void)
{
    return RADIO_ERROR_NOT_SUPPORTED;
}

int16_t sid_pal_radio_rssi(void)
{
    int8_t rssi = 0;

    return rssi;
}

int32_t sid_pal_radio_is_channel_free(uint32_t freq, int16_t threshold,
                                      uint32_t delay_us, bool *is_channel_free)
{
    int32_t err = RADIO_ERROR_NONE;
    uint32_t t_cur = 0;
    int16_t rssi;

    *is_channel_free = true;

    if ((err = sid_pal_radio_set_frequency(freq)) != RADIO_ERROR_NONE) {
        goto ret;
    }

    if ((err = sid_pal_radio_start_continuous_rx()) != RADIO_ERROR_NONE) {
        goto ret;
    }

    if (delay_us < SX130X_MIN_CHANNEL_NOISE_DELAY_US) {
        delay_us = SX130X_MIN_CHANNEL_NOISE_DELAY_US;
    }

    do {
        sid_pal_delay_us(SX130X_MIN_CHANNEL_NOISE_DELAY_US);
        rssi = sid_pal_radio_rssi();

        if (rssi > threshold) {
            *is_channel_free = false;
            break;
        }
        t_cur += SX130X_MIN_CHANNEL_NOISE_DELAY_US;
    } while (delay_us > t_cur);

ret:
    err = sid_pal_radio_standby();
    return err;
}

int32_t sid_pal_radio_random(uint32_t *random)
{
    int8_t seed, cnt = 0;
    int16_t rssi;
    uint32_t noise = 0, r_val = 0;
    int32_t err = RADIO_ERROR_NONE;

    *random = UINT32_MAX;

    if ((err = sid_pal_radio_start_continuous_rx()) != RADIO_ERROR_NONE) {
        goto ret;
    }

    do {
        sid_pal_delay_us(SX130X_MIN_CHANNEL_NOISE_DELAY_US);
        rssi = sid_pal_radio_rssi();

        noise += (-1 * rssi);
        seed = (noise % 8);
        r_val |= (((noise % 16) & 0xf) << (((seed + cnt) % 8) * 4));
        r_val |= noise;
        cnt++;
    } while (SX130X_RF_RANDOM_TIMES > cnt);
    *random = r_val;

ret:
    err = sid_pal_radio_standby();
    return err;
}

int16_t sid_pal_radio_get_ant_dbi(void)
{
    return drv_ctx.sx130x_cfg->ant_dbi;
}

int32_t sid_pal_radio_get_cca_level_adjust(sid_pal_radio_data_rate_t data_rate, int8_t *adj_level)
{
    if (data_rate <= SID_PAL_RADIO_DATA_RATE_INVALID || data_rate > SID_PAL_RADIO_DATA_RATE_MAX_NUM) {
        return RADIO_ERROR_INVALID_PARAMS;
    }
    *adj_level = drv_ctx.sx130x_cfg->cca_level_adjust[data_rate - 1];

    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_get_chan_noise(uint32_t freq, int16_t *noise)
{
   int32_t err;

    if ((err = sid_pal_radio_set_frequency(freq)) != RADIO_ERROR_NONE) {
        goto ret;
    }

    if ((err = sid_pal_radio_start_continuous_rx()) != RADIO_ERROR_NONE) {
        goto ret;
    }

    for (uint8_t i = 0; i < SX130X_NOISE_SAMPLE_SIZE; i++) {
        sid_pal_delay_us(SX130X_MIN_CHANNEL_NOISE_DELAY_US);
        *noise += sid_pal_radio_rssi();
    }
    *noise /= SX130X_NOISE_SAMPLE_SIZE;

ret:
    err = sid_pal_radio_standby();
    return err;
}

int32_t sid_pal_radio_get_radio_state_transition_delays(sid_pal_radio_state_transition_timings_t *state_delay)
{
    *state_delay = drv_ctx.sx130x_cfg->state_timings;
    return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_init(sid_pal_radio_event_notify_t notify,
                           sid_pal_radio_irq_handler_t dio_irq_handler,
                           sid_pal_radio_rx_packet_t *rx_packet)
{
    int32_t err = RADIO_ERROR_NONE;

    do {
        if (notify == NULL || dio_irq_handler == NULL || rx_packet == NULL) {
            err = RADIO_ERROR_INVALID_PARAMS;
            goto ret;
        }
        drv_ctx.radio_rx_packet = rx_packet;
        drv_ctx.report_radio_event = notify;
        drv_ctx.irq_handler = dio_irq_handler;
        drv_ctx.modem = SID_PAL_RADIO_MODEM_MODE_LORA;

        if ((err = radio_sx130x_platform_init()) != RADIO_ERROR_NONE) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            goto ret;
        }

        if ((err = radio_sx130x_set_radio_mode(false, false)) != RADIO_ERROR_NONE) {
            err = RADIO_ERROR_HARDWARE_ERROR;
            goto ret;
        }
        drv_ctx.radio_state = SID_PAL_RADIO_UNKNOWN;

        if ((err = sid_pal_radio_standby()) != RADIO_ERROR_NONE) {
            goto ret;
        }
    } while (0);

ret:
    if (err != RADIO_ERROR_NONE) {
        drv_ctx.radio_rx_packet = NULL;
        drv_ctx.report_radio_event = NULL;
    }

    return err;
}

int32_t sid_pal_radio_deinit(void)
{
    return RADIO_ERROR_NONE;
}
