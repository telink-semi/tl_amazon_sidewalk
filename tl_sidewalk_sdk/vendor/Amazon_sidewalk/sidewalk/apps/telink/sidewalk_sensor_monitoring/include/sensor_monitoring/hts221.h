/********************************************************************************************************
 * @file    hts221.h
 *
 * @brief   This is the header file for Telink RISC-V MCU
 *
 * @author  Driver Group
 * @date    2024
 *
 * @par     Copyright (c) 2024, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#ifndef HTS221__H
#define HTS221__H
/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C"
{
#endif
#include "tl_common.h"

#define HTS221_I2C_ADDR       0xBE
#define HTS221_WHO_AM_I       0x0F
#define HTS221_AV_CONF        0x10
#define HTS221_CTRL_REG1      0x20
#define HTS221_CTRL_REG2      0x21
#define HTS221_CTRL_REG3      0x22
#define HTS221_STATUS_REG     0x27
#define HTS221_HUMIDITY_OUT_L 0x28
#define HTS221_HUMIDITY_OUT_H 0x29
#define HTS221_TEMP_OUT_L     0x2A
#define HTS221_TEMP_OUT_H     0x2B

#define HTS221_H0_rH_x2       0x30
#define HTS221_H1_rH_x2       0x31
#define HTS221_T0_degC_x8     0x32
#define HTS221_T1_degC_x8     0x33
#define HTS221_T1_T0          0x35
#define HTS221_H0_T0_OUT_L    0x36
#define HTS221_H0_T0_OUT_H    0x37
#define HTS221_H1_T0_OUT_L    0x3A
#define HTS221_H1_T0_OUT_H    0x3B
#define HTS221_T0_OUT_L       0x3C
#define HTS221_T0_OUT_H       0x3D
#define HTS221_T1_OUT_L       0x3E
#define HTS221_T1_OUT_H       0x3F

    typedef struct
    {
        double humidity;
        double temperature;
    } hts221_data_st;

    uint8_t        hts221_iic_init(void);
    hts221_data_st hts221_iic_get_data(void);

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
#endif
