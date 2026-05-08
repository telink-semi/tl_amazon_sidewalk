/********************************************************************************************************
 * @file    sid_pal_serial_bus_telink_spi.c
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
#if 1 //CONFIG_SIDEWALK_SUBGHZ_SUPPORT
// sid_pal_serial_bus_telink_spi.c

#include "tl_common.h"
#include "drivers.h"

#include <sid_pal_serial_bus_ifc.h>
#include <sid_error.h>

#include <sid_pal_serial_bus_telink_spi.h>

static telink_spi_bus_t s_bus;

/* Calculate the div frequency index */
_attribute_ram_code_ static unsigned short div_from_speed(uint32_t hz) {
    // please use sys_clk.pll_clk, the real frequency need to mul 1000000
    const uint32_t pll_hz = sys_clk.pll_clk * 1000000u;
    uint32_t div = hz ? (pll_hz + hz - 1)/hz : 0;
    if (!div) div = 1;
    if (div > 0xFFFF) div = 0xFFFF;
    return (unsigned short)div;
}

/* ========== Reconfigure SPI on Demand ========== */
_attribute_ram_code_ static void apply_cfg_if_needed(uint32_t speed,
                                uint8_t mode,
                                enum sid_pal_serial_bus_bit_order ord)
{
    if (s_bus.cur_speed_hz == speed &&
        s_bus.cur_mode     == (mode & 0x3) &&
        s_bus.cur_order    == ord) {
        return;
    }

    spi_master_init(s_bus.spi_sel, div_from_speed(speed), (spi_mode_type_e)(mode & 0x3));
    spi_master_config(s_bus.spi_sel, SPI_NORMAL);
    spi_set_bit_sequence(s_bus.spi_sel,
        (ord == SID_PAL_SERIAL_BUS_BIT_ORDER_LSB_FIRST) ? SPI_LSB : SPI_MSB);

    s_bus.cur_speed_hz = speed;
    s_bus.cur_mode     = (mode & 0x3);
    s_bus.cur_order    = ord;
}

/* ========== Chip select assist: Prioritize callback, then hardware CS, and finally GPIO CS ========== */
static inline void cs_select_if_needed(const struct sid_pal_serial_bus_client *client)
{
    if (client->client_selector_cb) {
        (void)client->client_selector_cb(
            client, SID_PAL_SERIAL_BUS_CLIENT_SELECT, client->client_selector_context);
    } else if (!s_bus.use_hw_csn && s_bus.gpio_cs != GPIO_NONE_PIN) {
        gpio_function_en(s_bus.gpio_cs);
        gpio_output_en(s_bus.gpio_cs);
        gpio_input_dis(s_bus.gpio_cs);
        gpio_set_low_level(s_bus.gpio_cs);   // low level is valid
    }
}
static inline void cs_deselect_if_needed(const struct sid_pal_serial_bus_client *client)
{
    if (client->client_selector_cb) {
        (void)client->client_selector_cb(
            client, SID_PAL_SERIAL_BUS_CLIENT_DESELECT, client->client_selector_context);
    } else if (!s_bus.use_hw_csn && s_bus.gpio_cs != GPIO_NONE_PIN) {
        gpio_set_high_level(s_bus.gpio_cs);  // release
    }
}

/* ========== xfer Full-duplex ========== */
_attribute_ram_code_ static sid_error_t telink_spi_xfer(const struct sid_pal_serial_bus_iface *iface,
                                   const struct sid_pal_serial_bus_client *client,
                                   uint8_t *tx, uint8_t *rx, size_t xfer_size)
{
    (void)iface;
    if (!s_bus.inited || !client || (!tx && !rx) || xfer_size == 0) {
        return SID_ERROR_INVALID_ARGS;
    }

    apply_cfg_if_needed(client->speed_hz ? client->speed_hz : s_bus.cur_speed_hz,
                        client->mode,
                        client->bit_order);

    cs_select_if_needed(client);

    uint8_t tx_dummy[xfer_size];
    uint8_t rx_dummy[xfer_size];
    for(int i = 0; i < xfer_size; i++) {
        tx_dummy[i] = 0;
        rx_dummy[i] = 0;
    }

    spi_master_write_read_full_duplex(s_bus.spi_sel,
                                      tx ? tx : tx_dummy,
                                      rx ? rx : rx_dummy,
                                      (unsigned int)xfer_size);

    cs_deselect_if_needed(client);
    return SID_ERROR_NONE;
}

/* ========== xfer_hd is a half-duplex system (write-then-read / write-only / read-only) ========== */
_attribute_ram_code_ static sid_error_t telink_spi_xfer_hd(const struct sid_pal_serial_bus_iface *iface,
                                      const struct sid_pal_serial_bus_client *client,
                                      uint8_t *tx, uint8_t *rx,
                                      size_t tx_size, size_t rx_size)
{
    (void)iface;
    if (!s_bus.inited || !client || ((!tx || !tx_size) && (!rx || !rx_size))) {
        return SID_ERROR_INVALID_ARGS;
    }

    apply_cfg_if_needed(client->speed_hz ? client->speed_hz : s_bus.cur_speed_hz,
                        client->mode,
                        client->bit_order);

    cs_select_if_needed(client);

    sid_error_t ret = SID_ERROR_NONE;

    if (tx && tx_size && rx && rx_size) {
        if (spi_master_write_read(s_bus.spi_sel, tx, (unsigned int)tx_size,
                                  rx, (unsigned int)rx_size) != 0) {
            ret = SID_ERROR_IO_ERROR;
        }
    } else if (tx && tx_size) {
        if (spi_master_write(s_bus.spi_sel, tx, (unsigned int)tx_size) != 0) {
            ret = SID_ERROR_IO_ERROR;
        }
    } else if (rx && rx_size) {
        spi_master_read(s_bus.spi_sel, rx, (unsigned int)rx_size);
    }

    cs_deselect_if_needed(client);
    return ret;
}

/* ========== destroy ========== */
_attribute_ram_code_ static sid_error_t telink_spi_destroy(const struct sid_pal_serial_bus_iface *iface)
{
    (void)iface;
    if (!s_bus.inited) {
        return SID_ERROR_NONE;
    }
    spi_hw_fsm_reset(s_bus.spi_sel);
    s_bus.inited = false;
    return SID_ERROR_NONE;
}

/* ========== vtable ========== */
static const struct sid_pal_serial_bus_iface s_bus_vtbl = {
    .xfer    = telink_spi_xfer,
    .xfer_hd = telink_spi_xfer_hd,
    .destroy = telink_spi_destroy,
};

/* ========== create: IO initialization + gspi_set_pin + SPI initialization ========== */
sid_error_t sid_pal_serial_bus_telink_spi_create(const struct sid_pal_serial_bus_iface **iface,
                                                 const void *cfg_void)
{
    if (!iface || !cfg_void) return SID_ERROR_INVALID_ARGS;
    const struct sid_pal_telink_spi_bus_config *cfg =
        (const struct sid_pal_telink_spi_bus_config *)cfg_void;

    memset(&s_bus, 0, sizeof(s_bus));
    s_bus.spi_sel = GSPI_MODULE; // only support one spi

    // 1) Multiplexing pins, must be done before spi_master_init.
    gspi_set_pin((gspi_pin_config_t *)&cfg->pinmap);

    // 2) Determine if there is a hardware CSN, not recommended for Sx1262
    s_bus.use_hw_csn = (cfg->pinmap.spi_csn_pin != GPIO_NONE_PIN);
    s_bus.gpio_cs    = cfg->fallback_gpio_cs; // 允许在无硬件 CSN 时指定一个 GPIO 做 CS

    // 3) Initialize SPI parameters
    s_bus.cur_speed_hz = 0xFFFFFFFF;
    s_bus.cur_mode     = 0xFF;
    s_bus.cur_order    = 0xFF;
    apply_cfg_if_needed(cfg->init_speed_hz, cfg->init_mode, cfg->init_bit_order);

    s_bus.vtbl   = s_bus_vtbl;
    s_bus.inited = true;
    *iface       = &s_bus.vtbl;

    // 4)If GPIO CS is required (no hardware CSN and fallback_gpio_cs is set)
    if (!s_bus.use_hw_csn && s_bus.gpio_cs != GPIO_NONE_PIN) {
        gpio_set_high_level(s_bus.gpio_cs); // High for Idle state
        gpio_function_en(s_bus.gpio_cs);
        gpio_output_en(s_bus.gpio_cs);
        gpio_input_dis(s_bus.gpio_cs);

    }

    return SID_ERROR_NONE;
}


sid_error_t telink_factory_create(const struct sid_pal_serial_bus_iface **iface,
                                         const void *cfg_void)
{
    return sid_pal_serial_bus_telink_spi_create(iface, cfg_void);
}


struct sid_pal_serial_bus_factory telink_spi_bus_factory = {
    .create = telink_factory_create,
    .config = NULL,
};

#endif
