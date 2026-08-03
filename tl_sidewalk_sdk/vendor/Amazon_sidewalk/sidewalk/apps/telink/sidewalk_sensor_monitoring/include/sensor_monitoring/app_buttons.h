#pragma once
#include "types.h"
#include "gpio.h"
#include "app_config.h"

#define BUTTON1     (0)
//#define BUTTON2     (2)

typedef struct {
    uint8_t buttonID;
    gpio_pin_e button_GPIO;
    bool isButtonPressed;
} button_t;


void app_consume_all_buttons(void);

int app_is_any_button_pressed(void);

uint8_t app_get_pressed_buttons(uint8_t *list);

void app_consume_buttons(uint8_t *list, uint8_t numberOfButtons);

uint8_t app_get_button_IDs(uint8_t *list);

void app_register_button_press(uint32_t btn_id);

int app_buttons_init(void);


bool link_type_switch_button_check(void);

void link_type_switch_button_consume(void);

void app_buttons_pool(void);