#pragma once
#include <stdint.h>
#include "tl_common.h"
#include "app_config.h"


int app_sensor_temperature_get(int16_t *temp);
int app_sensors_init(void);
int app_sensors_recover_after_deep_sleep(void);

#define SIMULATED_SENSOR_LOWER_TEMP   (23)
#define SIMULATED_SENSOR_UPPER_TEMP   (25)
