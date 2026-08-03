#include <stdint.h>
#include <stdbool.h>
#include "gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sid_pal_gpio_ifc.h"
#include <sensor_monitoring/app_buttons.h>
#include "tl_common.h"
#include "sid_ble_adapter.h"


 _attribute_data_retention_ static button_t ButtonState[APP_NUMBER_OF_BUTTONS] = {
    { BUTTON1, BUTTON1_GPIO, false }
};


// ------- Critical section wrappers for thread safe

static inline UBaseType_t crit_enter_from_isr(void)
{
    return taskENTER_CRITICAL_FROM_ISR();
}

static inline void crit_exit_from_isr(UBaseType_t saved)
{
    taskEXIT_CRITICAL_FROM_ISR(saved);
}

static inline void crit_enter(void)
{
    taskENTER_CRITICAL();
}

static inline void crit_exit(void)
{
    taskEXIT_CRITICAL();
}

// ---------------------------------------------------



// -------------------- Only for Link Type swithing,

_attribute_data_retention_  bool linkTypeSwitchPressed = false;
void link_type_switch_button_register(void)
{
    linkTypeSwitchPressed = true;
}

bool link_type_switch_button_check(void)
{
    return linkTypeSwitchPressed;
}
void link_type_switch_button_consume(void)
{
    linkTypeSwitchPressed = false;
}

// --------------------------------------



void app_consume_all_buttons(void) {
    uint8_t cnt = 0;

    crit_enter();
    for(cnt=0; cnt!=APP_NUMBER_OF_BUTTONS; cnt++) {
        ButtonState[cnt].isButtonPressed = false;
    }
    crit_exit();
}

int app_is_any_button_pressed(void) {
    uint8_t cnt = 0, pressed = 0;

    crit_enter();
    for(cnt=0; cnt!=APP_NUMBER_OF_BUTTONS; cnt++) {
        if(ButtonState[cnt].isButtonPressed == true) {
            pressed = 1;
            break;
        }
    }
    crit_exit();
    return pressed;
}

uint8_t app_get_pressed_buttons(uint8_t *list) {
    uint8_t cnt_1 = 0, cnt_2=0;

    if(list == NULL) {
        TL_LOG_I("[BTN DRV] Buffer not provided!");
        return 0;
    }

    crit_enter();
    for(cnt_1=0; cnt_1!=APP_NUMBER_OF_BUTTONS; cnt_1++) {
        if(ButtonState[cnt_1].isButtonPressed == true) {
            list[cnt_2++] = ButtonState[cnt_1].buttonID;
        }
    }
    crit_exit();
    return cnt_2;
}

void app_consume_buttons(uint8_t *list, uint8_t numberOfButtons) {
    uint8_t cnt_1 = 0;

    if(list == NULL) {
        TL_LOG_I("[BTN DRV] List to consume not provided!");
        return;
    }

    crit_enter();
    for(cnt_1=0; cnt_1!=numberOfButtons; cnt_1++) {
        ButtonState[cnt_1].isButtonPressed = false;
    }
    crit_exit();
}

uint8_t app_get_button_IDs(uint8_t *list) {
    uint8_t cnt_1 = 0;

    if(list == NULL) {
        TL_LOG_I("[BTN DRV] Buffer not provided!");
        return 0;
    }

    crit_enter();
    for(cnt_1=0; cnt_1!=APP_NUMBER_OF_BUTTONS; cnt_1++) {
        list[cnt_1] = ButtonState[cnt_1].buttonID;
    }
    crit_exit();
    return cnt_1;
}

static void app_register_button_press_from_ISR(gpio_pin_e btnGPIO) {
    uint8_t cnt_1;

    UBaseType_t saved = crit_enter_from_isr();
    for(cnt_1=0; cnt_1!=APP_NUMBER_OF_BUTTONS; cnt_1++) {
        if(ButtonState[cnt_1].button_GPIO == btnGPIO) {
            ButtonState[cnt_1].isButtonPressed = true;
        }
    }
    crit_exit_from_isr(saved);
}

int app_buttons_init(void) {
    // configure as inputs
    gpio_function_en(BUTTON1_GPIO);
    gpio_output_dis(BUTTON1_GPIO);
    gpio_input_en(BUTTON1_GPIO);
    gpio_function_en(BUTTON2_GPIO);
    gpio_output_dis(BUTTON2_GPIO);
    gpio_input_en(BUTTON2_GPIO);

    // configure 10k pull-up for reliable button handling
    gpio_function_en(BUTTON1_GPIO);
    gpio_set_up_down_res(BUTTON1_GPIO, GPIO_PIN_PULLUP_10K);
    gpio_function_en(BUTTON2_GPIO);
    gpio_set_up_down_res(BUTTON2_GPIO, GPIO_PIN_PULLUP_10K);


    // configure interrupts
    gpio_set_irq(GPIO_IRQ5, BUTTON1_GPIO, INTR_FALLING_EDGE);
    gpio_set_irq(GPIO_IRQ5, BUTTON2_GPIO, INTR_FALLING_EDGE);

    gpio_set_irq_mask(GPIO_IRQ_IRQ5);
    plic_interrupt_enable(IRQ_GPIO_IRQ5);

    // configure wakup
    pm_set_gpio_wakeup(BUTTON1_GPIO,WAKEUP_LEVEL_LOW, 1);
    pm_set_gpio_wakeup(BUTTON2_GPIO,WAKEUP_LEVEL_LOW, 1);
    blc_pm_setWakeupSource(PM_WAKEUP_PAD);
    return 0;
}

_attribute_ram_code_sec_ void irq_button_handler(void)
{
    if (gpio_get_level(BUTTON1_GPIO) == 0) {
        app_register_button_press_from_ISR(BUTTON1_GPIO);
    }

    if (gpio_get_level(BUTTON2_GPIO) == 0) {
        link_type_switch_button_register();
    }
}

void app_buttons_pool(void) {
    irq_button_handler();
}
