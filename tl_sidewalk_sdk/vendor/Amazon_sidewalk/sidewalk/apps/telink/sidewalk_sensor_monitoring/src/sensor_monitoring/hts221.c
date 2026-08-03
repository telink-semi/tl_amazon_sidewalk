/********************************************************************************************************
 * @file    hts221.c
 *
 * @brief   This is the source file for Telink RISC-V MCU
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
#include "sensor_monitoring/hts221.h"
#include "sensor_monitoring/app_err.h"
#include "sensor_monitoring/app_sensor.h"
#include "tl_common.h"
#include "sid_ble_adapter.h"

static uint8_t hts221_iic_read(uint8_t reg_addr, uint8_t *pData, uint32_t Size)
{
    i2c_master_write(I2C_FOR_SENSORS, HTS221_I2C_ADDR, &reg_addr, 1);
    return i2c_master_read(I2C_FOR_SENSORS, HTS221_I2C_ADDR, pData, Size) ? 0 : ENXIO;
}

static uint8_t hts221_iic_write(uint8_t reg_addr, uint8_t pData)
{
    uint8_t write_buff[2] = {reg_addr, pData};
    return i2c_master_write(I2C_FOR_SENSORS, HTS221_I2C_ADDR, write_buff, 2) ? 0 : ENXIO;
}

uint8_t H0_rH_x2;
uint8_t H1_rH_x2;
uint8_t T0_degC_x8;
uint8_t T1_degC_x8;
uint8_t T1_T0;
uint8_t H0_T0_OUT_L;
uint8_t H0_T0_OUT_H;
uint8_t H1_T0_OUT_L;
uint8_t H1_T0_OUT_H;
uint8_t T0_OUT_L;
uint8_t T0_OUT_H;
uint8_t T1_OUT_L;
uint8_t T1_OUT_H;


double   H0;
double   H1;
uint16_t H0out;
uint16_t H1out;
double   T0;
double   T1;
int16_t  T0out;
int16_t  T1out;

uint8_t hts221_iic_init(void)
{
    uint8_t id  = 0;
    uint8_t ret = hts221_iic_read(HTS221_WHO_AM_I, &id, 1);
    if (!(0xbc == id && 0 == ret)) {
        return ENXIO;
    }

    // Temperature average samples = 16, humidity average samples = 32(0x1B)
    hts221_iic_write(HTS221_AV_CONF, 0x1b); //default
    // Select control register1(0x20) Power on, block data update, data rate o/p = 1 Hz(0x85)
    hts221_iic_write(HTS221_CTRL_REG1, 0x87);

    // Temperature Calibration values
    // Read 1 byte of data from address(0x32)
    hts221_iic_read(HTS221_T0_degC_x8, &T0_degC_x8, 1);
    // Read 1 byte of data from address(0x33)
    hts221_iic_read(HTS221_T1_degC_x8, &T1_degC_x8, 1);
    // Read 1 byte of data from address(0x35)
    hts221_iic_read(HTS221_T1_T0, &T1_T0, 1);
    TL_LOG_I("1    : %x \r\n", T0_degC_x8);
    TL_LOG_I("2    : %x \r\n", T1_degC_x8);
    TL_LOG_I("3    : %x \r\n", T1_T0);
    TL_LOG_I("4    : %x \r\n", (((T1_T0 & 0x03) << 8) | T0_degC_x8));
    TL_LOG_I("5    : %x \r\n", (((T1_T0 & 0x0C) << 6) | T1_degC_x8));
    // Convert the temperature Calibration values to 10-bits
    T0 = (((T1_T0 & 0x03) << 8) | T0_degC_x8) / 8.0;
    T1 = (((T1_T0 & 0x0C) << 6) | T1_degC_x8) / 8.0;

    //Read 1 byte of data from address(0x3C)
    hts221_iic_read(HTS221_T0_OUT_L, &T0_OUT_L, 1);
    //Read 1 byte of data from address(0x3D)
    hts221_iic_read(HTS221_T0_OUT_H, &T0_OUT_H, 1);
    //Read 1 byte of data from address(0x3E)
    hts221_iic_read(HTS221_T1_OUT_L, &T1_OUT_L, 1);
    //Read 1 byte of data from address(0x3F)
    hts221_iic_read(HTS221_T1_OUT_H, &T1_OUT_H, 1);
    T0out = (T0_OUT_H << 8) | T0_OUT_L;
    T1out = (T1_OUT_H << 8) | T1_OUT_L;
    TL_LOG_I("1    : %x \r\n", T0_OUT_L);
    TL_LOG_I("2    : %x \r\n", T0_OUT_H);
    TL_LOG_I("3    : %x \r\n", T1_OUT_L);
    TL_LOG_I("4    : %x \r\n", T1_OUT_H);

    // Read Calibration values from the non-volatile memory of the device
    // Humidity Calibration values
    // Read 1 byte of data from address(0x30)
    hts221_iic_read(HTS221_H0_rH_x2, &H0_rH_x2, 1);
    // Read 1 byte of data from address(0x31)
    hts221_iic_read(HTS221_H1_rH_x2, &H1_rH_x2, 1);
    H0 = H0_rH_x2 / 2.0;
    H1 = H1_rH_x2 / 2.0;

    //Read 1 byte of data from address(0x36)
    hts221_iic_read(HTS221_H0_T0_OUT_L, &H0_T0_OUT_L, 1);
    //Read 1 byte of data from address(0x37)
    hts221_iic_read(HTS221_H0_T0_OUT_H, &H0_T0_OUT_H, 1);
    //Read 1 byte of data from address(0x3A)
    hts221_iic_read(HTS221_H1_T0_OUT_L, &H1_T0_OUT_L, 1);
    //Read 1 byte of data from address(0x3B)
    hts221_iic_read(HTS221_H1_T0_OUT_H, &H1_T0_OUT_H, 1);
    H0out = (H0_T0_OUT_H << 8) | H0_T0_OUT_L;
    H1out = (H1_T0_OUT_H << 8) | H1_T0_OUT_L;

    TL_LOG_I("T0    : %.2f \r\n", T0);
    TL_LOG_I("T1    : %.2f \r\n", T1);
    TL_LOG_I("T0out : %d      \r\n", T0out);
    TL_LOG_I("T1out : %d   \r\n", T1out);
    TL_LOG_I("H0    : %.2f \r\n", H0);
    TL_LOG_I("H1    : %.2f \r\n", H1);
    TL_LOG_I("H0out : %d   \r\n", H0out);
    TL_LOG_I("H1out : %d   \r\n", H1out);
    return 0;
}

hts221_data_st hts221_iic_get_data(void)
{
    volatile uint32_t BBB1;
    volatile uint32_t BBB2;

    uint8_t        data_buff[4];
    hts221_data_st hts221_data;
    do {
        hts221_iic_read(HTS221_STATUS_REG, data_buff, 1);
        delay_ms(1);
    } while ((data_buff[0] & 0x03) != 0x03);

    hts221_iic_read(HTS221_HUMIDITY_OUT_L, data_buff, 1);
    hts221_iic_read(HTS221_HUMIDITY_OUT_H, &data_buff[1], 1);
    BBB1 = ((data_buff[1] << 8) | (data_buff[0]));

    hts221_iic_read(HTS221_TEMP_OUT_L, data_buff, 1);
    hts221_iic_read(HTS221_TEMP_OUT_H, &data_buff[1], 1);
    BBB2 = ((data_buff[1] << 8) | (data_buff[0]));
    if (BBB2 > 0x7fff) {
        BBB2 -= 0x10000;
    }

    double humidity    = (((1.0 * H1) - (1.0 * H0)) * (1.0 * BBB1 - 1.0 * H0out)) / (1.0 * H1out - 1.0 * H0out) + (1.0 * H0);
    double temperature = (((1.0 * T1) - (1.0 * T0)) * (1.0 * BBB2 - 1.0 * T0out)) / (1.0 * T1out - 1.0 * T0out) + (1.0 * T0);
    //    printf("humidity    : %.2f \r\n", humidity);
    //    printf("temperature : %.2fC \r\n", temperature);

    hts221_data.humidity    = humidity;
    hts221_data.temperature = temperature;
    return hts221_data;
}
