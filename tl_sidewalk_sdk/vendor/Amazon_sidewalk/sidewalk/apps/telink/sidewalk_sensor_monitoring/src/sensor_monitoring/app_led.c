#include "sensor_monitoring/app_leds.h"
#include <sensor_monitoring/app_leds.h>
#include "tl_common.h"
#include "sid_ble_adapter.h"


_attribute_ble_data_retention_ static LED_t LEDsState[APP_NUMBER_OF_LEDS] = {
    { LED0, LED1_GPIO, false },
    { LED1, LED2_GPIO, false }
};


static void app_LED_refresh(void) {
    uint8_t cnt;

    for(cnt=0; cnt!=APP_NUMBER_OF_LEDS; cnt++) {
        gpio_set_level(LEDsState[cnt].GPIOassigned, LEDsState[cnt].LEDstate);
    }
}

void app_LED_all_ON(void) {
    uint8_t cnt;
    for(cnt=0; cnt!=APP_NUMBER_OF_LEDS; cnt++) {
        LEDsState[cnt].LEDstate = true;
    }
    app_LED_refresh();
}

void app_LED_all_OFF(void) {
    uint8_t cnt;
    for(cnt=0; cnt!=APP_NUMBER_OF_LEDS; cnt++) {
        LEDsState[cnt].LEDstate = false;
    }
    app_LED_refresh();
}

void app_set_LED_selective(bool state, uint8_t *id, uint8_t numOfLEDs) {
    uint8_t cnt_1, cnt_2;

    for(cnt_1=0; cnt_1!=numOfLEDs; cnt_1++) {
        for(cnt_2=0; cnt_2!=APP_NUMBER_OF_LEDS; cnt_2++) {
            if(LEDsState[cnt_2].LEDid == id[cnt_1]) {
                LEDsState[cnt_2].LEDstate = state;
            }
        }
    }
    app_LED_refresh();
}

uint8_t app_LED_get_number_of_active(void) {
    uint8_t cnt, active = 0;
    for(cnt=0; cnt!=APP_NUMBER_OF_LEDS; cnt++) {
        if(LEDsState[cnt].LEDstate == true) {
            active++;
        }
    }
    return active;
}

uint8_t app_LED_get_selective(bool state, uint8_t *LEDList) {
    uint8_t cnt_1=0, cnt_2=0;
    if(LEDList == NULL) {
        TL_LOG_E("[LED driver] No buffer provided for app_LED_get_active");
        return 0;
    }

    for(cnt_1=0; cnt_1!=APP_NUMBER_OF_LEDS; cnt_1++) {
        if(LEDsState[cnt_1].LEDstate == state) {
            LEDList[cnt_2++] = LEDsState[cnt_1].LEDid;
            TL_LOG_E("[LED driver] FOUND!!");
        }
    }
    return cnt_2;
}

uint8_t app_LED_get_IDs(uint8_t *LEDList) {
    uint8_t cnt_1, cnt_2;
    if(LEDList == NULL) {
        TL_LOG_E("[LED driver] No buffer provided for app_LED_get_active");
        return 0;
    }

    for(cnt_1=0; cnt_1!=APP_NUMBER_OF_LEDS; cnt_1++) {
        LEDList[cnt_1] = (uint8_t) LEDsState[cnt_1].LEDid;
    }
    return APP_NUMBER_OF_LEDS;
}

int app_LED_init() {
    gpio_function_en(LED1_GPIO);
    gpio_function_en(LED2_GPIO);
    gpio_output_en(LED1_GPIO);
    gpio_output_en(LED2_GPIO);
    return 0;
}

void app_LED_recover_after_deep_sleep(void) {
    app_LED_init();
    app_LED_refresh();
}

void app_LED_single_on(uint8_t led) {

    app_set_LED_selective(true, &led, 1);

}

void app_LED_single_off(uint8_t led) {
    app_set_LED_selective(false, &led, 1);
}


// Only for debug:
void led_toggle(void) {
    gpio_toggle(LED1);
}