#include "sensor_monitoring/app_sensor.h"
#include "sensor_monitoring/hts221.h"
#include "sensor_monitoring/lsm9ds1.h"
#include "sensor_monitoring/app_err.h"
#include "tl_common.h"
#include "sid_ble_adapter.h"

int app_sensor_temperature_get(int16_t *temp) {
#if(APP_USE_REAL_SENSORS)
    hts221_data_st hts221_result = hts221_iic_get_data();
    *temp = hts221_result.temperature;
    return 0;
#else
    static uint32_t cnt = SIMULATED_SENSOR_LOWER_TEMP;
    static int dir = 1;
    *temp = cnt;
    cnt+=dir;

    if(cnt>= SIMULATED_SENSOR_UPPER_TEMP) dir = -1;
    if(cnt<= SIMULATED_SENSOR_LOWER_TEMP) dir = 1;
    TL_LOG_I("[SEN] Temp. reading:%d", *temp);
    return 0;
#endif
}

int app_sensor_IMU_get(uint8_t *IMU_buffer)
{
    if(IMU_buffer == NULL) {
        return -EIO;
    }
    lsm9ds1_data_st retData = lsm9ds1_iic_get_data();
    TL_LOG_I("[SEN] acc_x:%d",  retData.acc_x);
    TL_LOG_I("[SEN] acc_y:%d",  retData.acc_y);
    TL_LOG_I("[SEN] acc_z:%d",  retData.acc_z);

    IMU_buffer[0] =  (uint16_t)retData.acc_x & 0xFF;
    IMU_buffer[1] = ((uint16_t)retData.acc_x >> 8) & 0xFF;

    IMU_buffer[2] = (uint16_t)retData.acc_y & 0xFF;
    IMU_buffer[3] = ((uint16_t)retData.acc_y >> 8) & 0xFF;

    IMU_buffer[4] = (uint16_t)retData.acc_z & 0xFF;
    IMU_buffer[5] = ((uint16_t)retData.acc_z >> 8) & 0xFF;
    return 0;
}


int app_sensors_init_i2c(void) {
    i2c_set_pin(I2C_FOR_SENSORS, I2C_SDA_PIN, I2C_SCL_PIN);
    i2c_master_init(I2C_FOR_SENSORS);
    i2c_set_master_clk(I2C_FOR_SENSORS, (unsigned char)(sys_clk.pclk * 1000 * 1000 / (4 * I2C_CLK_SPEED)));
    return 0;
}

int app_sensors_init(void) {
#if(APP_USE_REAL_SENSORS)
    int retVal;
    // Init I2C
    app_sensors_init_i2c();

    // Init Temperature/humidity sensor
    retVal = hts221_iic_init();
    if(retVal != 0) {
        TL_LOG_E("[SEN]Failed to init HTS221 sensor! ERR:%d", retVal);
        return -EIO;
    }

    // Init IMU
    retVal = lsm9ds1_iic_init();
    if(retVal != 0) {
        TL_LOG_E("[SEN]Failed to init LSM9DS1 sensor! ERR:%d", retVal);
        return -EIO;
    }

#endif
    return 0;
}

int app_sensors_recover_after_deep_sleep(void) {
    app_sensors_init_i2c();
    return 0;
}