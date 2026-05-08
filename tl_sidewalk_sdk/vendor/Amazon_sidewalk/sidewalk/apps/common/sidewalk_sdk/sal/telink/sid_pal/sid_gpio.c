/********************************************************************************************************
 * @file    sid_gpio.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    11,2025
 *
 * @par     Copyright (c) 2025, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
/** @file sid_gpio.c
 *  @brief GPIO interface implementation.
 */
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "sid_ble_adapter.h"
#include <sid_pal_gpio_ifc.h>


#include <sid_pal_gpio_ifc.h>
#include <sid_pal_assert_ifc.h>
#include "lk/list.h"
#include "app_mem.h"

#if config_HW_SELECT
#define P0_PIN_NUM GPIO_PE2
#define P1_PIN_NUM GPIO_PE1
#else
#define P0_PIN_NUM GPIO_PD3
#define P1_PIN_NUM GPIO_PA4
#endif
u32 gpio_tbl[] = {P0_PIN_NUM, P1_PIN_NUM};
gpio_irq_num_e gpio_irq_tbl[] = {GPIO_IRQ0, GPIO_IRQ2};
gpio_irq_e gpio_irq_mask_tbl[] = {GPIO_IRQ_IRQ0, GPIO_IRQ_IRQ2};
u32 gpio_irq_src_tbl[] = {IRQ_GPIO_IRQ0, IRQ_GPIO_IRQ2};

#if (GPIO_COUNT == 1)
#define NUMBER_OF_PINS (P0_PIN_NUM)
#elif (GPIO_COUNT == 2)
#define NUMBER_OF_PINS (P0_PIN_NUM + P1_PIN_NUM)
#else
#error "Not supported."
#endif

static struct {
    list_node_t gpio_irq_list;
} irq_list = {
    .gpio_irq_list = LIST_INITIAL_VALUE(irq_list.gpio_irq_list),
};

typedef struct gpio_irq_context {
    list_node_t node;
    uint32_t gpio_number;
    sid_pal_gpio_irq_handler_t callback;
    void * callback_arg;
} gpio_irq_context_t;

static gpio_irq_context_t * get_irq_context(uint32_t gpio_number);

#if (FREERTOS_ENABLE)
#define   gpio_calloc(n, x)  pvPortMalloc(n*x)
#define   gpio_free(ptr)     vPortFree(ptr)
#else
#define   gpio_calloc(n, x)  app_malloc_nonreten(n*x)
#define   gpio_free(ptr)     app_free_nonreten(ptr)
#endif
#define  GPIO_INVALID_IRQ_NO  0xFF

volatile unsigned int gpio_irq1_cnt = 0;
void gpio_irq2_handler(void);

#if (FREERTOS_ENABLE)
PLIC_ISR_REGISTER_OS(gpio_irq2_handler, GPIO_IRQ_IRQ2);
#else
PLIC_ISR_REGISTER(gpio_irq2_handler, GPIO_IRQ_IRQ2);
#endif

_attribute_ram_code_ void gpio_irq1_handler(void)
{
    gpio_pin_e pin = gpio_tbl[0];
    gpio_irq_context_t *irq_context = get_irq_context(pin);

    SID_PAL_ASSERT(irq_context != NULL);
    if (irq_context->callback) {
        irq_context->callback(irq_context->gpio_number, irq_context->callback_arg);
    }
    gpio_clr_irq_status(GPIO_IRQ_IRQ0);
}

_attribute_ram_code_ void gpio_irq2_handler(void)
{
    gpio_pin_e pin = gpio_tbl[1];
    gpio_irq_context_t *irq_context = get_irq_context(pin);

    SID_PAL_ASSERT(irq_context != NULL);
    if (irq_context->callback) {
        irq_context->callback(irq_context->gpio_number, irq_context->callback_arg);
    }
    gpio_clr_irq_status(GPIO_IRQ_IRQ2);
}


static gpio_irq_context_t * get_irq_context(uint32_t gpio_number)
{
    gpio_irq_context_t *irq_context     = NULL;
    gpio_irq_context_t *ret_irq_context = NULL;

    list_for_every_entry(&irq_list.gpio_irq_list, irq_context, gpio_irq_context_t, node) {
        if (gpio_number == irq_context->gpio_number) {
            ret_irq_context = (gpio_irq_context_t *)&irq_context->node;
            break;
        }
    }

    return ret_irq_context;
}

static uint8_t get_irq_no(uint32_t gpio_number)
{
    gpio_irq_context_t *irq_context     = NULL;
    uint8_t irq_no = 0;

    list_for_every_entry(&irq_list.gpio_irq_list, irq_context, gpio_irq_context_t, node) {
        if (gpio_number == irq_context->gpio_number) {
            return irq_no;
            break;
        }
        irq_no ++;
    }
    return GPIO_INVALID_IRQ_NO;
}

static void create_irq_context(uint32_t gpio_number, sid_pal_gpio_irq_handler_t callback, void * callback_arg)
{
    gpio_irq_context_t *irq_context;
    if ((irq_context = gpio_calloc(1, sizeof(gpio_irq_context_t))) != NULL) {
        irq_context->gpio_number  = gpio_number;
        irq_context->callback     = callback;
        irq_context->callback_arg = callback_arg;
        list_add_tail(&irq_list.gpio_irq_list, &(irq_context->node));
    }
}

static void release_irq_context(uint32_t gpio_number)
{
    gpio_irq_context_t *irq_context = NULL;
    gpio_irq_context_t *irq_context_tmp = NULL;

    list_for_every_entry_safe(&irq_list.gpio_irq_list, irq_context, irq_context_tmp, gpio_irq_context_t, node) {
        if (gpio_number == irq_context->gpio_number) {
            list_delete(&irq_context->node);
            gpio_free(irq_context);
            break;
        }
    }
}

static sid_error_t gpio_pins_check(uint32_t gpio_number)
{
    return SID_ERROR_NONE;
}


_attribute_ram_code_ sid_error_t sid_pal_gpio_set_direction(uint32_t gpio_number,
                                       sid_pal_gpio_direction_t direction)
{
    if (gpio_pins_check(gpio_number) != SID_ERROR_NONE) {
        return SID_ERROR_NOSUPPORT;
    }
    if (direction > SID_PAL_GPIO_DIRECTION_OUTPUT) {
        return SID_ERROR_INVALID_ARGS;
    }
    gpio_pin_e pin = (gpio_pin_e)gpio_number;   //shouble be careful in app

    gpio_function_en(pin);

    if (direction == SID_PAL_GPIO_DIRECTION_INPUT) {
        gpio_output_dis(pin);
        gpio_input_en(pin);
    } else {
        gpio_input_dis(pin);
        gpio_output_en(pin);
    }


    return SID_ERROR_NONE;
}

_attribute_ram_code_ sid_error_t sid_pal_gpio_set_direction_to_high(uint32_t gpio_number,
                                       sid_pal_gpio_direction_t direction)
{
    if (gpio_pins_check(gpio_number) != SID_ERROR_NONE) {
        return SID_ERROR_NOSUPPORT;
    }
    if (direction > SID_PAL_GPIO_DIRECTION_OUTPUT) {
        return SID_ERROR_INVALID_ARGS;
    }
    gpio_pin_e pin = (gpio_pin_e)gpio_number;   //shouble be careful in app

    gpio_function_en(pin);

    if (direction == SID_PAL_GPIO_DIRECTION_INPUT) {
        gpio_output_dis(pin);
        gpio_input_en(pin);
    } else {
        gpio_set_level(pin,1);
        gpio_output_en(pin);
        gpio_input_dis(pin);

    }


    return SID_ERROR_NONE;
}

_attribute_ram_code_  sid_error_t sid_pal_gpio_input_mode(uint32_t gpio_number, sid_pal_gpio_input_t mode)
{
    if (gpio_pins_check(gpio_number) != SID_ERROR_NONE) {
        return SID_ERROR_NOSUPPORT;
    }
    if (mode > SID_PAL_GPIO_INPUT_DISCONNECT) {
        return SID_ERROR_INVALID_ARGS;
    }
    gpio_pin_e pin = (gpio_pin_e)gpio_number;   //shouble be careful in app

    gpio_function_en(pin);

    gpio_output_dis(pin);
    gpio_input_en(pin);


    return SID_ERROR_NONE;
}

_attribute_ram_code_ sid_error_t sid_pal_gpio_output_mode(uint32_t gpio_number, sid_pal_gpio_output_t mode)
{
    if (gpio_pins_check(gpio_number) != SID_ERROR_NONE) {
        return SID_ERROR_NOSUPPORT;
    }
    if (mode > SID_PAL_GPIO_OUTPUT_OPEN_DRAIN) {
        return SID_ERROR_INVALID_ARGS;
    }

    gpio_pin_e pin = (gpio_pin_e)gpio_number;   //shouble be careful in app

    gpio_function_en(pin);

    gpio_input_dis(pin);
    gpio_output_en(pin);

    return SID_ERROR_NONE;
}

_attribute_ram_code_  sid_error_t sid_pal_gpio_pull_mode(uint32_t gpio_number, sid_pal_gpio_pull_t pull)
{
    if (gpio_pins_check(gpio_number) != SID_ERROR_NONE) {
        return SID_ERROR_NOSUPPORT;
    }
    if (pull > SID_PAL_GPIO_PULL_DOWN) {
        return SID_ERROR_INVALID_ARGS;
    }
    gpio_pin_e pin = (gpio_pin_e)gpio_number;   //shouble be careful in app
    gpio_pull_type_e up_down_res = GPIO_PIN_UP_DOWN_FLOAT;
    switch (pull) {
    case SID_PAL_GPIO_PULL_NONE:
        up_down_res = GPIO_PIN_UP_DOWN_FLOAT;
        break;
    case SID_PAL_GPIO_PULL_UP:
        up_down_res = GPIO_PIN_PULLUP_10K; //GPIO_PIN_PULLUP_1M  todo
        break;
    case SID_PAL_GPIO_PULL_DOWN:
        up_down_res =  GPIO_PIN_PULLDOWN_100K;
        break;
    }

    gpio_set_up_down_res(pin,up_down_res);
    return SID_ERROR_NONE;
}

_attribute_ram_code_  sid_error_t sid_pal_gpio_read(uint32_t gpio_number, uint8_t *value)
{
    if (gpio_pins_check(gpio_number) != SID_ERROR_NONE) {
        return SID_ERROR_NOSUPPORT;
    }
    gpio_pin_e pin = (gpio_pin_e)gpio_number;   //shouble be careful in app
    *value = gpio_get_level(pin);

    return SID_ERROR_NONE;
}

_attribute_ram_code_ sid_error_t sid_pal_gpio_write(uint32_t gpio_number, uint8_t value)
{
    if (gpio_pins_check(gpio_number) != SID_ERROR_NONE) {
        return SID_ERROR_NOSUPPORT;
    }
    gpio_pin_e pin = (gpio_pin_e)gpio_number;   //Note the definitions within the APP
    gpio_set_level(pin, value);

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_gpio_toggle(uint32_t gpio_number)
{
    if (gpio_pins_check(gpio_number) != SID_ERROR_NONE) {
        return SID_ERROR_NOSUPPORT;
    }
    gpio_pin_e pin = (gpio_pin_e)gpio_number;   //Note the definitions within the APP
    gpio_toggle(pin);

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_gpio_set_irq(uint32_t gpio_number, sid_pal_gpio_irq_trigger_t irq_trigger,
                                     sid_pal_gpio_irq_handler_t gpio_callback, void * callback_arg)
{
    if (gpio_pins_check(gpio_number) != SID_ERROR_NONE) {
        return SID_ERROR_NOSUPPORT;
    }
    if (irq_trigger > SID_PAL_GPIO_IRQ_TRIGGER_HIGH) {
        return SID_ERROR_INVALID_ARGS;
    } else if (irq_trigger > SID_PAL_GPIO_IRQ_TRIGGER_EDGE) {
        return SID_ERROR_NOT_FOUND;
    }

    gpio_pin_e pin = (gpio_pin_e)gpio_number;   //Note the definitions within the APP
    gpio_irq_trigger_type_e trigger_type;
    switch(irq_trigger)
    {
    case SID_PAL_GPIO_IRQ_TRIGGER_NONE:
    break;
    case SID_PAL_GPIO_IRQ_TRIGGER_RISING:
        trigger_type = INTR_RISING_EDGE;
        break;
    case SID_PAL_GPIO_IRQ_TRIGGER_FALLING:
        trigger_type = INTR_FALLING_EDGE;
        break;
    case SID_PAL_GPIO_IRQ_TRIGGER_EDGE:
        trigger_type = INTR_RISING_EDGE;   //todo !!!!!!
        break;
    case SID_PAL_GPIO_IRQ_TRIGGER_LOW:
        trigger_type = INTR_LOW_LEVEL;
        break;
    case SID_PAL_GPIO_IRQ_TRIGGER_HIGH:
        trigger_type = INTR_HIGH_LEVEL;
        break;
    default:
        break;
    }
    if (get_irq_context(gpio_number)) {
        release_irq_context(gpio_number);
    }
    uint8_t irq_no = 0;
    gpio_irq_num_e irq = gpio_irq_tbl[irq_no];


    if (irq_trigger != SID_PAL_GPIO_IRQ_TRIGGER_NONE) {
        create_irq_context(gpio_number, gpio_callback, callback_arg);

        uint8_t irq_no = get_irq_no(gpio_number);
        if(GPIO_INVALID_IRQ_NO == irq_no)
        {
            return SID_ERROR_PARAM_OUT_OF_RANGE;
        }
        irq = gpio_irq_tbl[irq_no];
        gpio_pin_e pin = (gpio_pin_e)gpio_number;
        gpio_set_irq(irq, pin,trigger_type);
        gpio_irq_e  irq_mask = gpio_irq_mask_tbl[irq_no];
        gpio_set_irq_mask(irq_mask);
        plic_interrupt_enable(gpio_irq_src_tbl[irq_no]);

    }

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_gpio_irq_enable(uint32_t gpio_number)
{
    if (gpio_pins_check(gpio_number) != SID_ERROR_NONE) {
        return SID_ERROR_NOSUPPORT;
    }
    gpio_pin_e pin = (gpio_pin_e)gpio_number;   //Note the definitions within the APP

    uint8_t irq_no = get_irq_no(gpio_number);
    if(GPIO_INVALID_IRQ_NO == irq_no)
    {
        return SID_ERROR_PARAM_OUT_OF_RANGE;
    }
    gpio_irq_num_e irq = gpio_irq_tbl[irq_no];
    gpio_irq_en(pin,irq);

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_gpio_irq_disable(uint32_t gpio_number)
{
    if (gpio_pins_check(gpio_number) != SID_ERROR_NONE) {
        return SID_ERROR_NOSUPPORT;
    }

    gpio_pin_e pin = (gpio_pin_e)gpio_number;   //Note the definitions within the APP
    uint8_t irq_no = get_irq_no(gpio_number);
    if(GPIO_INVALID_IRQ_NO == irq_no)
    {
        return SID_ERROR_PARAM_OUT_OF_RANGE;
    }
    gpio_irq_num_e irq = gpio_irq_tbl[irq_no];
    gpio_irq_dis(pin,irq);   //

    return SID_ERROR_NONE;
}
