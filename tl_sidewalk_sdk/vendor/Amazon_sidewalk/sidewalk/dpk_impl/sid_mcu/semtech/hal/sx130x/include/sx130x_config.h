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

#ifndef SX130X_CONFIG_H
#define SX130X_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sid_time_types.h>
#include <sid_pal_serial_bus_ifc.h>
#include <semtech_radio_ifc.h>
#include <sid_pal_radio_ifc.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>

#define SEMTECH_ID_SX1301                       0x1
#define SEMTECH_ID_SX1302                       0x2
#define SEMTECH_ID_SX1303                       0x3

#define HALO_GPIO_NOT_CONNECTED                 128
#define SX130X_MAX_CHANNEL_NUM                  10
#define SX130X_MAX_RFCHAIN_NUM                  2
#define SX130x_TX_GAIN_LUT_SIZE_MAX             16

typedef enum
{
    RADIO_SX130X_REG_MODE_LDO  = 0x00,  //!< (Default)
    RADIO_SX130X_REG_MODE_DCDC = 0x01,
} radio_sx130x_reg_mode_t;

typedef enum {
    SX130X_TCXO_CTRL_NONE = 0,
    SX130X_TCXO_CTRL_VDD  = 1,
    SX130X_TCXO_CTRL_DIO3 = 2
} radio_sx130x_tcxo_ctrl_t;

typedef struct radio_sx130x_pa_cfg {
    uint8_t pa_duty_cycle;
    uint8_t hp_max;
    uint8_t device_sel;
    uint8_t pa_lut;
    int8_t tx_power;
    uint8_t ramp_time;
} radio_sx130x_pa_cfg_t;

typedef struct radio_sx130x_tcxo {
    radio_sx130x_tcxo_ctrl_t ctrl;
    uint8_t dio3_to_mcu_pin;
    uint8_t voltage;
    uint32_t timeout;
} radio_sx130x_tcxo_t;

/**
@brief Structure containing all gains of Tx chain
*/
struct radio_sx130x_tx_gain {
    int8_t  rf_power;   /*!> measured TX power at the board connector, in dBm */
    uint8_t dig_gain;   /*!> (sx125x) 2 bits: control of the digital gain of SX1302 */
    uint8_t pa_gain;    /*!> (sx125x) 2 bits: control of the external PA (SX1302 I/O)
                             (sx1250) 1 bits: enable/disable the external PA (SX1302 I/O) */
    uint8_t dac_gain;   /*!> (sx125x) 2 bits: control of the radio DAC */
    uint8_t mix_gain;   /*!> (sx125x) 4 bits: control of the radio mixer */
    int8_t offset_i;    /*!> (sx125x) calibrated I offset */
    int8_t offset_q;    /*!> (sx125x) calibrated Q offset */
    uint8_t pwr_idx;    /*!> (sx1250) 6 bits: control the radio power index to be used for configuration */
};

/**
@brief Structure defining the Tx gain LUT
*/
struct radio_sx130x_tx_gain_lut {
    struct radio_sx130x_tx_gain lut[SX130x_TX_GAIN_LUT_SIZE_MAX];
    uint8_t size;
};

typedef int32_t (*radio_sx130x_get_pa_cfg_t)(int8_t tx_power, radio_sx130x_pa_cfg_t *pa_cfg);

typedef struct {
    uint8_t                                 id;
    radio_sx130x_reg_mode_t                 regulator_mode;
    bool                                    rx_boost;
    int8_t                                  lna_gain;
    const radio_sx130x_get_pa_cfg_t         pa_cfg_callback;
    const struct sid_pal_serial_bus_factory *bus_factory;

    struct {
        uint32_t                            power;
        uint32_t                            rf_sw_ena;
        uint32_t                            txrx;
        uint32_t                            tx_bypass;
    }                                       gpios;

    struct sid_pal_serial_bus_client        bus_selector;
    float                                   rssi_offset[SX130X_MAX_RFCHAIN_NUM];
    struct {
        uint32_t                            sf;
        uint8_t                             bw;
        uint8_t                             crc;
    }                                       high_speed_lora;
    uint32_t                                channel_freq_hz;
    int32_t                                 channel_if[SX130X_MAX_CHANNEL_NUM];
    uint8_t                                 channel_rfchain[SX130X_MAX_CHANNEL_NUM];
    int8_t                                  max_tx_power[SID_PAL_RADIO_DATA_RATE_MAX_NUM];
    int8_t                                  max_tx_pwr;
    int8_t                                  min_tx_pwr;
    struct radio_sx130x_tx_gain_lut         txlut;
    radio_sx130x_tcxo_t                     tcxo;
    int16_t                                 ant_dbi;
    int8_t                                  cca_level_adjust[SID_PAL_RADIO_DATA_RATE_MAX_NUM];
    struct {
        uint8_t *p;
        size_t   size;
    } internal_buffer;

    sid_pal_radio_state_transition_timings_t state_timings;
} radio_sx130x_device_config_t;

/**
 * Callback to notify when radio is about to switch to sleep state
 * The callback is called in software irq context. The user has to
 * switch context from software irq in this callback.
 */
typedef void (*sid_pal_radio_sleep_start_notify_handler_t)(struct sid_timespec * const wakeup_time);

/** @brief Set Semtech radio config parameters
 *
 *  @param pointer to sx130x radio device config
 */
void set_radio_sx130x_device_config(const radio_sx130x_device_config_t *cfg);

/** @brief Get tx power range the semtech chip supports
 *  This API is used by diagnostics firmware only.
 *  This is used to determine the max tx power the chip supports so that the diag
 *  firmware will not exceed the max tx power setting the chip supports
 *
 *  @param  max tx power to be populated.
 *  @param  min tx power to be populated.
 *  @return On success RADIO_ERROR_NONE, on error a negative number is returned
 */
int32_t get_sx130x_tx_power_range(int8_t *max_tx_power, int8_t *min_tx_power);

#ifdef __cplusplus
}
#endif

#endif
