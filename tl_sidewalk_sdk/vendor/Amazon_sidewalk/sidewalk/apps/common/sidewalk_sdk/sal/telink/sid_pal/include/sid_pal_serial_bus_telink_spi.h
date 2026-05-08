/********************************************************************************************************
 * @file    sid_pal_serial_bus_telink_spi.h
 *
 * @brief   This is the header file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2022
 *
 * @par     Copyright (c) 2022, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#ifndef SID_PAL_SERIAL_BUS_TELINK_SPI_H
#define SID_PAL_SERIAL_BUS_TELINK_SPI_H

/* hardware configuration, user will setting in the  */
struct sid_pal_telink_spi_bus_config {
    gspi_pin_config_t pinmap;                    // will use gspi_set_pin() set
    uint32_t  init_speed_hz;                     // init SPI speed
    uint8_t   init_mode;                         // (CPOL/CPHA)
    enum sid_pal_serial_bus_bit_order init_bit_order; // MSB / LSB
    gpio_pin_e fallback_gpio_cs;                 // use a specific GPIO for CS by default
};


typedef struct {
    struct sid_pal_serial_bus_iface vtbl;
    spi_sel_e spi_sel;
    bool inited;

    // The current hardware SPI is configured with parameters (used to determine whether reconfiguration is needed when switching devices).
    uint32_t  cur_speed_hz;
    uint8_t   cur_mode;
    enum sid_pal_serial_bus_bit_order cur_order;

    // CS selection strategy
    bool      use_hw_csn;        // pinmap.spi_csn_pin != GPIO_NONE_PIN
    gpio_pin_e gpio_cs;          // This GPIO is available when there is no hardware CSN and no CS control callback.
} telink_spi_bus_t;

sid_error_t telink_factory_create(const struct sid_pal_serial_bus_iface **iface,
                                         const void *cfg_void);


#endif
