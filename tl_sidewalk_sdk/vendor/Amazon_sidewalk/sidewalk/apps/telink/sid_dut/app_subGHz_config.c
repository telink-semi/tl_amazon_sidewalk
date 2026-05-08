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
#include <sx126x_config.h>
//#include <target/memory.h>

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
int pa_cfg_callback(int8_t req_dbm, radio_sx126x_pa_cfg_t *out)
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
    flash_read_page(app_get_trim_addr(),sizeof(data),&data);
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
    .gpio_tx_bypass  = GPIO_NONE_PIN, // 若有外置PA/FEM再填
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


_attribute_ram_code_ void app_sid_sleep_enter(u8 e, u8 *p, int n)
{
    gpio_set_up_down_res(RADIO_NSS,PM_PIN_PULLUP_10K);
    gpio_output_dis(RADIO_NSS);
    trng_disable();
}

_attribute_ram_code_ void app_sid_wakeup(u8 e, u8 *p, int n)
{
    gpio_function_en(RADIO_NSS);
    gpio_output_en(RADIO_NSS);
    gpio_input_dis(RADIO_NSS);
    gpio_set_high_level(RADIO_NSS);   // low level is valid
    gpio_analog_resistance_init();
    trng_enable();

}

void app_sleep_config(void)
{
    blc_ll_registerTelinkControllerEventCallback(BLT_EV_FLAG_SLEEP_ENTER, &app_sid_sleep_enter);
    blc_ll_registerTelinkControllerEventCallback(BLT_EV_FLAG_SUSPEND_EXIT, &app_sid_wakeup);
}
