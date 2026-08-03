/********************************************************************************************************
 * @file    lsm9ds1.c
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
#include "sensor_monitoring/lsm9ds1.h"
#include "sensor_monitoring/app_err.h"
#include "sensor_monitoring/app_sensor.h"
#include "tl_common.h"
#include "sid_ble_adapter.h"

static uint8_t lsm9ds1_ag_iic_read(uint8_t reg_addr, uint8_t *pData, uint32_t Size)
{
    i2c_master_write(I2C_FOR_SENSORS, LSM9DS1_I2C_AG_ADDR, &reg_addr, 1);
    return i2c_master_read(I2C_FOR_SENSORS, LSM9DS1_I2C_AG_ADDR, pData, Size) ? 0 : ENXIO;
}

static uint8_t lsm9ds1_mag_iic_read(uint8_t reg_addr, uint8_t *pData, uint32_t Size)
{
    i2c_master_write(I2C_FOR_SENSORS, LSM9DS1_I2C_MAG_ADDR, &reg_addr, 1);
    return i2c_master_read(I2C_FOR_SENSORS, LSM9DS1_I2C_MAG_ADDR, pData, Size) ? 0 : ENXIO;
}

__attribute__((unused)) static uint8_t lsm9ds1_ag_iic_write(uint8_t reg_addr, uint8_t pData)
{
    uint8_t write_buff[2] = {reg_addr, pData};
    return i2c_master_write(I2C_FOR_SENSORS, LSM9DS1_I2C_AG_ADDR, write_buff, 2) ? 0 : ENXIO;
}

__attribute__((unused)) static uint8_t lsm9ds1_mag_iic_write(uint8_t reg_addr, uint8_t pData)
{
    uint8_t write_buff[2] = {reg_addr, pData};
    return i2c_master_write(I2C_FOR_SENSORS, LSM9DS1_I2C_MAG_ADDR, write_buff, 2) ? 0 : ENXIO;
}

uint8_t lsm9ds1_iic_init(void)
{
    uint8_t ag_id   = 0;
    uint8_t mag_id  = 0;
    uint8_t data    = 0;
    uint8_t data1   = 0;
    uint8_t ag_ret  = lsm9ds1_ag_iic_read(LSM9DS1_WHO_AM_I, &ag_id, 1);
    uint8_t mag_ret = lsm9ds1_mag_iic_read(LSM9DS1_WHO_AM_I, &mag_id, 1);
    if (!(0x68 == ag_id && 0 == ag_ret && 0x3d == mag_id && 0 == mag_ret)) {
        return ENXIO;
    }

    lsm9ds1_ag_iic_read(LSM9DS1_CTRL_REG8, &data, 1);
    data |= BIT(0); // SWRESET
    do {
        lsm9ds1_ag_iic_write(LSM9DS1_CTRL_REG8, data);
        lsm9ds1_ag_iic_read(LSM9DS1_CTRL_REG8, &data1, 1);
        delay_ms(2);
    } while (data1 & BIT(0));

    // Configure gyroscope: 119 Hz, ±245 dps
    lsm9ds1_ag_iic_write(LSM9DS1_CTRL_REG1_G, 0x63);
    // Configure accelerometer
    lsm9ds1_ag_iic_write(LSM9DS1_CTRL_REG6_XL, 0xC8);

    // Configure magnetometer: 80Hz, Continuous mode
    lsm9ds1_mag_iic_write(LSM9DS1_CTRL_REG1_M, 0xDC); // Temperature enabled,80Hz
    lsm9ds1_mag_iic_write(LSM9DS1_CTRL_REG2_M, 0x00); // ±4 Gauss
    lsm9ds1_mag_iic_write(LSM9DS1_CTRL_REG3_M, 0x00); // Continuous conversion mode
    return 0;
}

lsm9ds1_data_st lsm9ds1_iic_get_data(void)
{
    uint8_t         data_buff[6];
    lsm9ds1_data_st lsm9ds1_data;
    lsm9ds1_ag_iic_read(LSM9DS1_STATUS_REG, data_buff, 1);
    lsm9ds1_data.statue = data_buff[0];
    //    do{
    //        lsm9ds1_ag_iic_read(LSM9DS1_STATUS_REG,data_buff,1);
    //        delay_ms(1);
    //    } while ((data_buff[0] & 0x03) != 0x03);
    lsm9ds1_ag_iic_read(LSM9DS1_OUT_TEMP_L, data_buff, 2);
    lsm9ds1_data.temperature = ((data_buff[1] << 8) | data_buff[0]) & 0xfff;
    if (lsm9ds1_data.temperature > 0x7FFF) {
        lsm9ds1_data.temperature -= 0x10000;
    }
    lsm9ds1_data.temperature = (lsm9ds1_data.temperature / 16) + 25;

    lsm9ds1_ag_iic_read(LSM9DS1_OUT_X_G, data_buff, 6);
    lsm9ds1_data.pitch_x = (int16_t)((data_buff[1] << 8) | data_buff[0]);
    lsm9ds1_data.pitch_y = (int16_t)((data_buff[3] << 8) | data_buff[2]);
    lsm9ds1_data.pitch_z = (int16_t)((data_buff[5] << 8) | data_buff[4]);

    lsm9ds1_ag_iic_read(LSM9DS1_OUT_X_XL, data_buff, 6);
    lsm9ds1_data.acc_x = (int16_t)((data_buff[1] << 8) | data_buff[0]);
    lsm9ds1_data.acc_y = (int16_t)((data_buff[3] << 8) | data_buff[2]);
    lsm9ds1_data.acc_z = (int16_t)((data_buff[5] << 8) | data_buff[4]);

    lsm9ds1_mag_iic_read(LSM9DS1_OUT_X_L_M, data_buff, 6);
    lsm9ds1_data.mag_x = (int16_t)((data_buff[1] << 8) | data_buff[0]);
    lsm9ds1_data.mag_y = (int16_t)((data_buff[3] << 8) | data_buff[2]);
    lsm9ds1_data.mag_z = (int16_t)((data_buff[5] << 8) | data_buff[4]);
    return lsm9ds1_data;
}
