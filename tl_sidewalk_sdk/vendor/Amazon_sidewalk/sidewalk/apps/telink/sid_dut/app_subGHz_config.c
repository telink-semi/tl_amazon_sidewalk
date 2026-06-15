/*
 * Copyright 2020-2023 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "sid_ble_adapter.h"
#include <sid_api.h>
#include <sid_error.h>
#include <sidewalk.h>
#include <sx126x_config.h>
#include <sx126x.h>

#include <sid_pal_serial_bus_ifc.h>
#include <sid_pal_radio_ifc.h>
#include <sid_pal_serial_bus_telink_spi.h>
#include <sid_pal_gpio_ifc.h>
#include <sid_pal_uptime_ifc.h>

#include <sx126x_config.h>
#include <sx126x_radio.h>

#include "app_subGHz_config.h"

u32 app_get_trim_addr(void);

/*
 * Sx1262 Radio SPI Config
 */


/* This product has no external PA and SX1262 can support max of 22dBm*/
#define RADIO_SX1262_MAX_TX_POWER                                  22
#define RADIO_SX1262_MIN_TX_POWER                                  -9

#define RADIO_MAX_TX_POWER_NA                                      20
#define RADIO_MAX_TX_POWER_EU                                      14

#if defined (REGION_ALL)
#define RADIO_REGION                                               RADIO_REGION_NONE
#elif defined (REGION_US915)
#define RADIO_REGION                                               RADIO_REGION_NA
#elif defined (REGION_EU868)
#define RADIO_REGION                                               RADIO_REGION_EU
#endif

#define RADIO_SX1262_SPI_BUFFER_SIZE                               255

#define RADIO_SX1262_PA_DUTY_CYCLE                                 0x04
#define RADIO_SX1262_HP_MAX                                        0x07
#define RADIO_SX1262_DEVICE_SEL                                    0x00
#define RADIO_SX1262_PA_LUT                                        0x01

#define RADIO_RX_LNA_GAIN                                          0
#define RADIO_MAX_CAD_SYMBOL                                       SID_PAL_RADIO_LORA_CAD_04_SYMBOL
#define RADIO_ANT_GAIN(X)                                          ((X) * 100)

#define RADIO_DEFAULT_TRIM_CAP_VAL                                0x1212

const gspi_pin_config_t pinmap = {
    .spi_csn_pin      = GPIO_NONE_PIN,
    .spi_clk_pin      = RADIO_SCLK,
    .spi_mosi_io0_pin = RADIO_MOSI,
    .spi_miso_io1_pin = RADIO_MISO,
    .spi_io2_pin      = GPIO_NONE_PIN,
    .spi_io3_pin      = GPIO_NONE_PIN,
};

struct sid_pal_telink_spi_bus_config bus_cfg = {
    .pinmap           = pinmap,
    .init_speed_hz    = 8 * 1000 * 1000,
    .init_mode        = 0,
    .init_bit_order   = SID_PAL_SERIAL_BUS_BIT_ORDER_MSB_FIRST,
    .fallback_gpio_cs = RADIO_NSS,
};

const struct sid_pal_serial_bus_iface *bus = NULL;


struct sid_pal_serial_bus_client dev = {
    .speed_hz                = 8 * 1000 * 1000,
    .mode                    = 0,
    .bit_order               = SID_PAL_SERIAL_BUS_BIT_ORDER_MSB_FIRST,
    .client_selector_cb      = NULL,
    .client_selector_context = NULL,
};

static uint8_t                           s_spi_internal_buf[RADIO_SX1262_SPI_BUFFER_SIZE];

const struct sid_pal_serial_bus_factory telink_spi_bus_factory_for_dut = {
    .create = telink_factory_create,
    .config = &bus_cfg,
};

// --- App callback ---
void app_radio_event_notify(sid_pal_radio_events_t evt)
{

}

// irq callback
void app_radio_dio_irq_handler(void)
{
    return;
    //(void)sid_pal_radio_irq_process();  // put it in mainloop
}

// PA config callback
int32_t pa_cfg_callback(int8_t req_dbm, radio_sx126x_pa_cfg_t *out)
{
    out->pa_duty_cycle = 0x04;
    out->hp_max        = 0x07;
    out->device_sel    = 0x00;           //0x01; // 1262
    out->pa_lut        = 0x01;
    out->tx_power      = req_dbm;
    out->ramp_time     = 0x04; // 40us
    out->enable_ext_pa = false;
    return 0;
}

#if (CONFIG_DIO3_FOR_ANT_SW)

int32_t sx126x_dio3_output_voltage(uint8_t voltage);
int32_t sx126x_dio3_gpio_clear(void);
static int32_t radio_dio3_ctrl_voltage(uint8_t radio_state)
{
  int32_t err = SID_ERROR_NONE;

  if(radio_state == 0 || radio_state == 2)
  {
      if ((err = sx126x_dio3_output_voltage(RADIO_SX126X_TCXO_CTRL_3_3V)) != SID_ERROR_NONE)
      {
          TL_LOG_E("pal sx126x: sx126x_dio3_output_voltage error");
      }
  }
  else
  {
      if (sx126x_dio3_gpio_clear() != 0)
          TL_LOG_E("pal sx126x: sx126x_dio3_gpio_clear error");
  }
  return err;
}
#endif

int32_t app_sid_radio_sx126x_get_mfg_trim_val(uint16_t *trim)
{
    #define  APP_INVALID_TRIM_VALUE 0xFFFFFFFF
    if(NULL == trim)
        return -1;
    u32 data = APP_INVALID_TRIM_VALUE;
    flash_read_page(app_get_trim_addr(),sizeof(data),(uint8_t *)&data);
    if(data == APP_INVALID_TRIM_VALUE)
    {
        *trim = CONFIG_SIDEWALK_SID_SUBG_TRIM_VAL;
    }
    else
    {
        *trim = (data >>24) | ((data >>8) & 0xFF00);
    }
    TL_LOG_D("sug trim %x",*trim);
    return 0;
}

const radio_sx126x_regional_param_t radio_sx126x_regional_param[] = {
    {.param_region     = RADIO_REGION_NA,
     .max_tx_power     = {RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA, RADIO_MAX_TX_POWER_NA},
     .cca_level_adjust = {0, 0, 0, 0, 0, 0},
     .ant_dbi          = RADIO_ANT_GAIN(2.15)},
};


const radio_sx126x_device_config_t radio_sx1262_cfg = {
    .id              = SEMTECH_ID_SX1262, // chip id register not supported
    .regulator_mode  = RADIO_SX126X_REGULATOR_DCDC,
    .rx_boost        = false,
    .lna_gain        = 0,
    .bus_factory     = &telink_spi_bus_factory_for_dut,
    .gpio_radio_busy = RADIO_BUSY,
    .gpio_int1       = RADIO_DIO_1,
    .gpio_power      = RADIO_RESET,   // 若无电源控制脚就留 NOT_CONNECTED
    .gpio_tx_bypass  = HALO_GPIO_NOT_CONNECTED, // 若有外置PA/FEM再填
    .gpio_rf_sw_ena  = ANT_SWITCH_POWER,
    .trim_cap_val_callback = app_sid_radio_sx126x_get_mfg_trim_val,
    .pa_cfg_callback = pa_cfg_callback,
    #if (CONFIG_DIO3_FOR_ANT_SW )
    .dio3_cfg_callback          = radio_dio3_ctrl_voltage,
    #endif
    .bus_selector =
        {
            .client_selector = RADIO_NSS, // sx1262_NSS
            .speed_hz        = 8000000,
            .bit_order       = SID_PAL_SERIAL_BUS_BIT_ORDER_MSB_FIRST,
            .mode            = 0,
        },

    .tcxo =
        {
            .ctrl = SX126X_TCXO_CTRL_NONE,
            .dio3_to_mcu_pin = HALO_GPIO_NOT_CONNECTED,
        },

    .regional_config =
        {
            .radio_region         = RADIO_REGION_NA,
            .reg_param_table_size = sizeof(radio_sx126x_regional_param) / sizeof(radio_sx126x_regional_param[0]),
            .reg_param_table      = radio_sx126x_regional_param,
        },

    .state_timings =
        {
            .sleep_to_full_power_us = 406,
            .full_power_to_sleep_us = 0,
            .rx_to_tx_us            = 0,
            .tx_to_rx_us            = 0,
            .tx_delay_us            = 1200,
            .rx_delay_us            = 1500,
        },

    .internal_buffer =
        {
            .p    = s_spi_internal_buf,
            .size = sizeof(s_spi_internal_buf),
        },
};


const radio_sx126x_device_config_t* get_radio_cfg(void)
{
    return &radio_sx1262_cfg;
}

const struct sid_sub_ghz_links_config sub_ghz_link_config = {
    .enable_link_metrics = true,
    .sar_dcr = 100,
    .registration_config = {
        .enable = true,
        .periodicity_s = UINT32_MAX,
    },
    .link2_max_tx_power_in_dbm = RADIO_MAX_TX_POWER_NA,
    .link3_max_tx_power_in_dbm = RADIO_MAX_TX_POWER_NA,
};

const struct sid_sub_ghz_links_config* app_get_sub_ghz_config(void)
{
    return &sub_ghz_link_config;
}


void sx126x_bringup(void)
{
    //need to set maunally just now
//    telink_spi_bus_factory.config = &bus_cfg;

    //set_radio_sx126x_device_config(&radio_sx1262_cfg);

    //SID_ERROR_CHECK(sid_pal_radio_init(app_radio_event_notify, app_radio_dio_irq_handler, &g_rx));
}




#if (CONFIG_DIO3_FOR_ANT_SW)
/*
 * KGM100XB DVT1 use SX126X DIO3 to supply power for ANT SW.
 * Configuration:
 * 1. Set bit 3 of register@0x0580 (output enable on DIO3)
 * 2. Clear bit 3 of register@0x0583 (input disable on DIO3)
 * 3. Clear bit 3 of register@0x0584 (pull-up disable on DIO3) - optional
 * 4. Clear bit 3 of register@0x0585 (pull-down disable on DIO3) - optional
 * 5. Set bits [0 to 2] of register@0x0920 to the output voltage you need on DIO3
 *    (see Table 13-35: tcxoVoltage Configuration Definition in the related datasheet)
 *
 * Output programming:
 * - Set bit 3 of register@0x0920 to have DIO3 "high"
 * - Clear bit 3 of register@0x0920 to have DIO3 "low"
 */
int32_t sx126x_dio3_output_voltage(uint8_t voltage)
{
  halo_drv_semtech_ctx_t *ctx = sx126x_get_drv_ctx();
  int32_t err = SID_ERROR_GENERIC;
  uint8_t reg_val = 0;
  do {
//    SL_SID_LOG_PAL_INFO("pal sx126x: sx126x_dio3_output_voltage %d", voltage);

    reg_val = 0x1 << 3;
    // set bit 3 of 0x0580
    if (sx126x_read_register(ctx, 0x0580, &reg_val, 1) != SX126X_STATUS_OK) {
        fflush(NULL);
        TL_LOG_E("sx126x_read_register error %x",ctx);
      break;
    }
    reg_val |= (0x1 << 3);
    if (sx126x_write_register(ctx, 0x0580, &reg_val, 1) != SX126X_STATUS_OK) {
        fflush(NULL);
        TL_LOG_E("sx126x_write_register error %x",ctx);
      break;
    }
    // clear bit 3 of 0x0583
    if (sx126x_read_register(ctx, 0x0583, &reg_val, 1) != SX126X_STATUS_OK) {
        fflush(NULL);
          TL_LOG_E("sx126x_read_register error2 %x",ctx);
      break;
    }
    reg_val &= ~(0x1 << 3);
    if (sx126x_write_register(ctx, 0x0583, &reg_val, 1) != SX126X_STATUS_OK) {
        fflush(NULL);
        TL_LOG_E("sx126x_write_register error2 %x",ctx);
      break;
    }

    reg_val = voltage;     // Set voltage
    if (sx126x_write_register(ctx, 0x0920, &reg_val, 1) != SX126X_STATUS_OK) {
        fflush(NULL);
        TL_LOG_E("sx126x_read_register error3 %x",ctx);
      break;
    }

    // Set Output
    if (sx126x_read_register(ctx, 0x0920, &reg_val, 1) != SX126X_STATUS_OK) {
        fflush(NULL);
        TL_LOG_E("sx126x_read_register error4 %x",ctx);
      break;
    }
    reg_val |= (0x1 << 3);     // High output
    if (sx126x_write_register(ctx, 0x0920, &reg_val, 1) != SX126X_STATUS_OK) {
        fflush(NULL);
        TL_LOG_E("sx126x_write_register error3 %x",ctx);
      break;
    }
    err = SID_ERROR_NONE;
  } while (0);
  return 0;
}

int32_t sx126x_dio3_gpio_clear(void)
{
  int32_t status = SX126X_STATUS_OK;
  uint8_t reg_val = 0x00;
  halo_drv_semtech_ctx_t *ctx = sx126x_get_drv_ctx();
  // set bit 3 of 0x0920
  status = sx126x_read_register(ctx, 0x0920, &reg_val, 1);
  reg_val &= ~(1 << 3);
  status = sx126x_write_register(ctx, 0x0920, &reg_val, 1);

  return status;
}

#endif


static void radio_irq(uint32_t pin, void * callback_arg)
{
    (void)callback_arg;
    halo_drv_semtech_ctx_t *ctx = sx126x_get_drv_ctx();
    uint8_t pinState;
    if (sid_pal_gpio_read(pin, &pinState) == SID_ERROR_NONE) {
        if (pinState) {
            sid_pal_uptime_now(&(ctx->radio_rx_packet->rcv_tm));
            ctx->irq_handler();
        }
    }
}

int32_t sid_pal_radio_reinit(void)
{
    int32_t err = 0;
    halo_drv_semtech_ctx_t *ctx = sx126x_get_drv_ctx();
#ifdef BOARD_HAL_IO_EXPANDER_SUBG_BAND_PIN
        if (sid_pal_gpio_set_direction(BOARD_HAL_EXP_GPIO( BOARD_HAL_IO_EXPANDER_SUBG_BAND_PIN),
            SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            return RADIO_ERROR_IO_ERROR;
        }

        if (sid_pal_gpio_write(BOARD_HAL_EXP_GPIO( BOARD_HAL_IO_EXPANDER_SUBG_BAND_PIN), 0) != SID_ERROR_NONE) {
            return RADIO_ERROR_IO_ERROR;
        }
#endif

    if (ctx->config->gpio_radio_busy != HALO_GPIO_NOT_CONNECTED) {
        if (sid_pal_gpio_set_direction(ctx->config->gpio_radio_busy,
            SID_PAL_GPIO_DIRECTION_INPUT) != SID_ERROR_NONE) {
            return RADIO_ERROR_IO_ERROR;
        }
    }

    if (ctx->config->gpio_tx_bypass != HALO_GPIO_NOT_CONNECTED) {
        if (sid_pal_gpio_set_direction(ctx->config->gpio_tx_bypass,
            SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            return RADIO_ERROR_IO_ERROR;
        }
    }

    if (ctx->config->gpio_rf_sw_ena != HALO_GPIO_NOT_CONNECTED) {
        if (sid_pal_gpio_set_direction(ctx->config->gpio_rf_sw_ena,
            SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            return RADIO_ERROR_IO_ERROR;
        }
    }
    if (ctx->config->gpio_power != HALO_GPIO_NOT_CONNECTED) {
        if (sid_pal_gpio_set_direction_to_high(ctx->config->gpio_power,
            SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
            return RADIO_ERROR_IO_ERROR;
        }
    }

    if (ctx->config->pa_cfg_callback == NULL) {
        return RADIO_ERROR_IO_ERROR;
    }

    if (ctx->config->trim_cap_val_callback != NULL) {
        ctx->config->trim_cap_val_callback(&ctx->trim);
    } else {
        ctx->trim = RADIO_DEFAULT_TRIM_CAP_VAL;
    }

    if (ctx->config->bus_factory->create(&ctx->bus_iface, ctx->config->bus_factory->config) != SID_ERROR_NONE) {
        err = RADIO_ERROR_IO_ERROR;
        return RADIO_ERROR_IO_ERROR;
    }


    if (sid_pal_gpio_set_irq(ctx->config->gpio_int1,
            SID_PAL_GPIO_IRQ_TRIGGER_RISING, radio_irq, NULL) != SID_ERROR_NONE) {
        err = RADIO_ERROR_IO_ERROR;
        return RADIO_ERROR_IO_ERROR;
    }

    HAOJIE_DBG_CHN6_LOW;
    return err;
}
