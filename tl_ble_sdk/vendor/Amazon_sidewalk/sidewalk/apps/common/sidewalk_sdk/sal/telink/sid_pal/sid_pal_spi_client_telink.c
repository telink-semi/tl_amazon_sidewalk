/********************************************************************************************************
 * @file    sid_pal_spi_client_telink.c
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
#if CONFIG_SIDEWALK_SUBGHZ_SUPPORT
#include <sid_pal_serial_bus_ifc.h>
#include <sid_pal_serial_client_ifc.h>
#include <sid_error.h>

#include "tl_common.h"
#include "drivers.h"

//static telink_spi_client_t s_cli;
//
///* Calculate the div frequency index */
//static unsigned short div_from_speed(uint32_t hz) {
//    // please use sys_clk.pll_clk, the real frequency need to mul 1000000
//    const uint32_t pll_hz = sys_clk.pll_clk * 1000000u;
//    uint32_t div = hz ? (pll_hz + hz - 1)/hz : 0;
//    if (!div) div = 1;
//    if (div > 0xFFFF) div = 0xFFFF;
//    return (unsigned short)div;
//}
//
///* ========== send/get_frame/get_mtu/destroy ========== */
//static sid_error_t cli_send(const sid_pal_serial_ifc_t *_this,
//                            const uint8_t *frame, size_t size)
//{
//    (void)_this;
//    if (!frame || size == 0 || size > s_cli.mtu) {
//        return SID_ERROR_INVALID_ARGS;
//    }
//
//    if (s_cli.use_gpio_cs && s_cli.gpio_cs != GPIO_NONE_PIN) {
//        gpio_set_low_level(s_cli.gpio_cs);
//    }
//
//    sid_error_t ret = SID_ERROR_NONE;
//    if (spi_master_write(GSPI_MODULE, (unsigned char *)frame, (unsigned int)size) != 0) {
//        ret = SID_ERROR_IO_ERROR;
//    }
//
//    if (s_cli.use_gpio_cs && s_cli.gpio_cs != GPIO_NONE_PIN) {
//        gpio_set_high_level(s_cli.gpio_cs);
//    }
//
//    if (s_cli.cbs && s_cli.cbs->tx_done_cb) {
//        s_cli.cbs->tx_done_cb(s_cli.user_ctx);
//    }
//    return ret;
//}
//
//static sid_error_t cli_get_frame(const sid_pal_serial_ifc_t *_this,
//                                 uint8_t **frame, size_t *size)
//{
//    (void)_this; (void)frame; (void)size;
//    // For actual RX: use SPIS + DMA/interrupt + ring buffer, and call new_rx_done_cb() inside ISR/task
//    return SID_ERROR_NOSUPPORT;
//}
//
//static sid_error_t cli_get_mtu(const sid_pal_serial_ifc_t *_this, uint16_t *mtu)
//{
//    (void)_this;
//    if (!mtu) return SID_ERROR_INVALID_ARGS;
//    *mtu = s_cli.mtu;
//    return SID_ERROR_NONE;
//}
//
//static void cli_destroy(const sid_pal_serial_ifc_t *_this)
//{
//    (void)_this;
//    if (s_cli.master_inited_local) {
//        spi_hw_fsm_reset(GSPI_MODULE);
//        s_cli.master_inited_local = false;
//    }
//}
//
///* ========== vtable ========== */
//static const struct sid_pal_serial_ifc_s s_cli_vtbl = {
//    .send      = cli_send,
//    .get_frame = cli_get_frame,
//    .process   = NULL,
//    .get_mtu   = cli_get_mtu,
//    .destroy   = cli_destroy,
//};
//
///* ========== create: supports gspi_set_pin + SPI initialization ========== */
//sid_error_t sid_pal_spi_client_create(sid_pal_serial_ifc_t const **_this,
//                                      const void *cfg_void,
//                                      const sid_pal_serial_params_t *params)
//{
//    if (!_this || !cfg_void || !params) {
//        return SID_ERROR_INVALID_ARGS;
//    }
//
//    const struct sid_pal_telink_spi_client_config *cfg =
//        (const struct sid_pal_telink_spi_client_config *)cfg_void;
//
//    memset(&s_cli, 0, sizeof(s_cli));
//    s_cli.ifc      = s_cli_vtbl;
//    s_cli.cbs      = params->callbacks;
//    s_cli.user_ctx = params->user_ctx;
//    s_cli.mtu      = cfg->mtu ? cfg->mtu : 255;
//
//    // 1) Configure pin multiplexing
//    gspi_set_pin((gspi_pin_config_t *)&cfg->pinmap);
//
//    // 2) Initialize SPI master
//    unsigned short div = div_from_speed(cfg->speed_hz);
//    spi_master_init(GSPI_MODULE, div, (spi_mode_type_e)(cfg->mode & 0x3));
//    spi_master_config(GSPI_MODULE, SPI_NORMAL);
//    spi_set_bit_sequence(GSPI_MODULE,
//        (cfg->bit_order == SID_PAL_SERIAL_BUS_BIT_ORDER_LSB_FIRST) ? SPI_LSB : SPI_MSB);
//
//    // 3) If no hardware CSN and GPIO CS is allowed, set GPIO high as idle state
//    if (cfg->pinmap.spi_csn_pin == GPIO_NONE_PIN
//        && cfg->gpio_cs != GPIO_NONE_PIN) {
//        s_cli.use_gpio_cs = true;
//        s_cli.gpio_cs     = cfg->gpio_cs;
//        gpio_function_en(s_cli.gpio_cs);
//        gpio_set_func(s_cli.gpio_cs, AS_GPIO);
//        gpio_output_en(s_cli.gpio_cs);
//        gpio_input_dis(s_cli.gpio_cs);
//        gpio_set_high_level(s_cli.gpio_cs);
//    }
//
//    s_cli.master_inited_local = true;
//
//    *_this = (const sid_pal_serial_ifc_t *)&s_cli;
//    return SID_ERROR_NONE;
//}


#endif
