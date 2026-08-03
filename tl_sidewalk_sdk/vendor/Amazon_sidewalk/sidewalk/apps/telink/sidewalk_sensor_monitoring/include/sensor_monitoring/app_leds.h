#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "gpio.h"
#include "app_config.h"


#define LED0            (0)
#define LED1            (1)



typedef struct {
    uint8_t    LEDid;
    gpio_pin_e GPIOassigned;
    bool       LEDstate;
} LED_t;



void app_LED_all_ON(void);


void app_LED_all_OFF(void);


void app_set_LED_selective(bool state, uint8_t *id, uint8_t numOfLEDs);


uint8_t app_LED_get_selective(bool state, uint8_t *LEDList);


uint8_t app_LED_get_IDs(uint8_t *LEDList);

uint8_t app_LED_get_number_of_active(void);

int app_LED_init();

void app_LED_single_on(uint8_t led);

void app_LED_single_off(uint8_t led);

void app_LED_recover_after_deep_sleep(void);
// For debug ony
void led_toggle(void);

