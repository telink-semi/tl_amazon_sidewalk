/********************************************************************************************************
 * @file    lsm9ds1.h
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
#ifndef LSM9DS1_H
#define LSM9DS1_H
/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C"
{
#endif
#include "tl_common.h"

#define LSM9DS1_I2C_AG_ADDR  0xD6
#define LSM9DS1_I2C_MAG_ADDR 0x3C

#define LSM9DS1_WHO_AM_I     0x0F

#define LSM9DS1_CTRL_REG5_XL 0x1F
#define LSM9DS1_CTRL_REG6_XL 0x20
#define LSM9DS1_CTRL_REG1_G  0x10
#define LSM9DS1_CTRL_REG4    0x1E
#define LSM9DS1_CTRL_REG8    0x22 // Disable FIFO

#define LSM9DS1_CTRL_REG1_M  0x20
#define LSM9DS1_CTRL_REG2_M  0x21
#define LSM9DS1_CTRL_REG3_M  0x22

#define LSM9DS1_OUT_TEMP_L   0x15
#define LSM9DS1_OUT_X_G      0x18
#define LSM9DS1_OUT_X_XL     0x28
#define LSM9DS1_OUT_X_L_M    0x28

#define LSM9DS1_STATUS_REG   0x27

    typedef struct
    {
        uint8_t statue;
        int32_t temperature;
        int16_t pitch_x;
        int16_t pitch_y;
        int16_t pitch_z;
        int16_t acc_x;
        int16_t acc_y;
        int16_t acc_z;
        int16_t mag_x;
        int16_t mag_y;
        int16_t mag_z;
    } lsm9ds1_data_st;

    uint8_t         lsm9ds1_iic_init(void);
    lsm9ds1_data_st lsm9ds1_iic_get_data(void);

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
#endif
