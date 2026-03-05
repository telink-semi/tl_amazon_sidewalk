/********************************************************************************************************
 * @file    sidewalk_app.c
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
#include "tl_common.h"
#include "drivers.h"
#include "sid_ble_adapter.h"
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <sid_sdk_config.h>
#include <app.h>
//#include <sidewalk.h>
#include <app_ble_config.h>
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
#include <app_subGHz_config.h>
#endif
#include <sid_hal_reset_ifc.h>
//#include <sid_hal_memory_ifc.h>
#include <stdbool.h>
#include <bt_app_callbacks.h>
#include <sid_api.h>
#include <sid_pal_common_ifc.h>
#include <app_mfg_config.h>
#include "app_ui.h"


#include <sid_clock_ifc.h>
#include <sid_asd_cli.h>
#include <sid_config_cli.h>
#include <sid_qa.h>
#include <sid_device_information.h>
#include <sid_utils.h>

#include "app_mem.h"
#include "app_buffer.h"


#define KEY1  0x01
#define KEY2  0x2
#define KEY3  0xf1
#define KEY4  0xf0
#define PARAM_UNUSED (0U)

_attribute_ble_data_retention_ static uint32_t persistent_link_mask = SID_LINK_TYPE_1;  //ble
_attribute_ble_data_retention_ static uint8_t  sid_app_sleep_flag  = 0;  //ble


#define MAIN_TASK_STACK_SIZE        (4096 / sizeof(configSTACK_DEPTH_TYPE))
#define MSG_QUEUE_LEN 10
#define MSG_LOG_BLOCK_SIZE 80
#define DEVICE_INFO_ERROR 1
#define DEVICE_INFO_SUCCESS 0

#define SERIAL_NUM_SIZE 16
#define MAC_ADDRESS_SIZE 6
#define PRODUCT_FW_VERSION_SIZE 8
#define DEVICE_KIND_SIZE 22

typedef enum {
    LOG_BACKEND_DISABLE = 0,
    LOG_BACKEND_ENABLE = 1,
} log_backend_action_t;

typedef struct app_local_context {
    TaskHandle_t main_task;
} app_local_context_t;


static app_local_context_t app_context = {
    .main_task = NULL,
};

struct sid_end_device_characteristics dev_ch = {
    .type = SID_END_DEVICE_TYPE_STATIC,
    .power_type = SID_END_DEVICE_POWERED_BY_BATTERY_AND_LINE_POWER,
    .qualification_id = 0x0005,
};

static void pwr_meas_mode_enter()
{

}

static void pwr_meas_mode_exit()
{

}

static void pwr_meas_is_blocked(void)
{
#if FREERTOS_ENABLE
    // sid_qa processing is blocked and requires the owning RTOS task to yield
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
#else
  // for no os ,can be sleep ,so release  the flag
    sid_app_sleep_flag = 0;
#endif

}

static void pwr_meas_is_unblocked(void)
{

#if FREERTOS_ENABLE
    // sid_qa processing is unblocked therefore the owning RTOS task can be resumed
    if (xPortIsInsideInterrupt()) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(app_context.main_task, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    } else {
        xTaskNotifyGive(app_context.main_task);
    }
#else
    // for no os ,can be sleep ,so set  the flag
    sid_app_sleep_flag  =1;
#endif
}


bool app_sidewalk_can_sleep(void)
{
  return sid_app_sleep_flag?false:true;
}


extern char _end[];
extern uint32_t _STACK_TOP;
#define MEMORY_POOL_END    (((uint32_t)&_STACK_TOP) - 0x400)

void * _sbrk(ptrdiff_t incr)
{
  static uintptr_t heap_end;
  if (heap_end == 0) heap_end = (uintptr_t) _end;

  uintptr_t new_heap_end = heap_end + incr;

  if (new_heap_end > MEMORY_POOL_END) {
    errno = ENOMEM;
    return (void*) -1;
  }
  uintptr_t old_heap_end = heap_end;
  heap_end = new_heap_end;
  return (void*) old_heap_end;
}



static void main_thread(void *context)
{
    struct sid_config config = {
            .callbacks = NULL,
            .link_config = SID_SDK_CONFIG_ENABLE_LINK_TYPE_1 ? app_get_ble_config() : NULL,
    #if SID_SDK_CONFIG_ENABLE_LINK_TYPE_2 || SID_SDK_CONFIG_ENABLE_LINK_TYPE_3
            .sub_ghz_link_config = app_get_sub_ghz_config(),
    #endif
            .dev_ch = dev_ch,
        };

    struct sid_qa_pwr_meas_if pwr_meas_if = {
        .enter_func = pwr_meas_mode_enter,
        .exit_func = pwr_meas_mode_exit,
        .blocked_func = pwr_meas_is_blocked,
        .unblocked_func = pwr_meas_is_unblocked,
    };
    sid_qa_set_config(&config, &pwr_meas_if);
    TL_LOG_I("sid QA cli application started... %x",MEMORY_POOL_END);
    app_uart_init();
    while (1) {
         sid_cli_process();
         sid_qa_process(QA_PROC_NO_WAIT);
         void sid_timer_check_timer_start(void);
         sid_timer_check_timer_start();
         #if (TLKAPI_DEBUG_ENABLE)
         tlkapi_debug_handler();
         #endif
    }
error:
    sid_platform_deinit();
    vTaskDelete(NULL);
}


void Portble_btn_press(u8 key)
{
    if(KEY1 == key)
    {

    }
    else if(KEY2 == key)
    {

    }
    else    if(KEY3 == key)
    {

    }
    else
    {

    }
}

void Portble_btn_d_press(u8 key)
{

}
void Portble_btn_l_press(u8 key)
{

}



static void reboot_func(void)
{
    sys_reboot();
}


static void set_sub_ghz_cfg(struct sid_sub_ghz_links_config *sub_ghz_cfg)
{
#if SID_SDK_CONFIG_ENABLE_LINK_TYPE_3 || SID_SDK_CONFIG_ENABLE_LINK_TYPE_2
    if (!sub_ghz_cfg) {
        TL_LOG_E("Null pointer passed while setting sub ghz cfg");
    }
    struct sid_sub_ghz_links_config *cfg = app_get_sub_ghz_config();
    memcpy(cfg, sub_ghz_cfg, sizeof(*cfg));
#endif
}


uint8_t dsn[SERIAL_NUM_SIZE] = {0x47, 0x50, 0x31, 0x33, 0x53, 0x34, 0x30, 0x30,
                                0x35, 0x30, 0x39, 0x35, 0x56, 0x33, 0x57, 0x32};
uint8_t mac[MAC_ADDRESS_SIZE] = {0x9C, 0xC8, 0xE9, 0x95, 0xCC, 0x10};
// fw_ver = 1.85.0-4
uint8_t fw_ver[PRODUCT_FW_VERSION_SIZE] = {0x31, 0x2e, 0x38, 0x35, 0x2e, 0x30, 0x2d, 0x34};

uint8_t dev_kind[DEVICE_KIND_SIZE] = {0x72, 0x65, 0x66, 0x65, 0x72, 0x65, 0x6E, 0x63, 0x65, 0x5F, 0x62,
                                      0x6F, 0x61, 0x72, 0x64, 0x5F, 0x6E, 0x6F, 0x72, 0x64, 0x69, 0x63};

static struct sid_device_info dev_info = {
    .serial_number_size = SERIAL_NUM_SIZE,
    .mac_address_size = MAC_ADDRESS_SIZE,
    .product_fw_version_size = PRODUCT_FW_VERSION_SIZE,
    .device_kind_size = DEVICE_KIND_SIZE,
    .serial_number = dsn,
    .mac_address = mac,
    .product_fw_version = fw_ver,
    .device_kind = dev_kind,
};

int app_start(void)
{
    platform_parameters_t platform_parameters = {
            .mfg_store_region.addr_start = sid_mfg_get_start_addr(),
            .mfg_store_region.addr_end = sid_mfg_get_end_addr(),
#if SID_SDK_CONFIG_ENABLE_LINK_TYPE_3 || SID_SDK_CONFIG_ENABLE_LINK_TYPE_2
            .platform_init_parameters.radio_cfg = (radio_sx126x_device_config_t *)get_radio_cfg(),
#endif
    };
    app_sleep_config();
    sid_error_t ret_code = sid_platform_init(&platform_parameters);
    if (ret_code != SID_ERROR_NONE) {
        TL_LOG_E("Sidewalk Platform Init err: %d", ret_code);
         configASSERT(0);
    }
    sid_cli_init();
    sid_config_cli_init();

    struct sid_qa_callbacks qa_callbacks = {
        .reboot_cmd = reboot_func,
        .set_sub_ghz_cfg = set_sub_ghz_cfg,
        .device_info_cfg = &dev_info,
    };

    sid_qa_init(&qa_callbacks);


    if (pdPASS != xTaskCreate(main_thread, "sidewalk", MAIN_TASK_STACK_SIZE, &app_context, CONFIG_SIDEWALK_THREAD_PRIORITY, &app_context.main_task)) {
        TL_LOG_E("sidewalk xTaskCreate init  err");
         configASSERT(0);
    }

}


int app_sidewalk_init(void)
{
    app_uart_init();
    platform_parameters_t platform_parameters = {
            .mfg_store_region.addr_start = sid_mfg_get_start_addr(),
            .mfg_store_region.addr_end = sid_mfg_get_end_addr(),
#if SID_SDK_CONFIG_ENABLE_LINK_TYPE_3 || SID_SDK_CONFIG_ENABLE_LINK_TYPE_2
            .platform_init_parameters.radio_cfg = (radio_sx126x_device_config_t *)get_radio_cfg(),
#endif
    };
    app_sleep_config();
    sid_error_t ret_code = sid_platform_init(&platform_parameters);
    if (ret_code != SID_ERROR_NONE) {
        TL_LOG_E("Sidewalk Platform Init err: %d", ret_code);
         configASSERT(0);
    }
    sid_cli_init();
    sid_config_cli_init();

    struct sid_qa_callbacks qa_callbacks = {
        .reboot_cmd = reboot_func,
        .set_sub_ghz_cfg = set_sub_ghz_cfg,
        .device_info_cfg = &dev_info,
    };

    sid_qa_init(&qa_callbacks);

    struct sid_config config = {
            .callbacks = NULL,
            .link_config = SID_SDK_CONFIG_ENABLE_LINK_TYPE_1 ? app_get_ble_config() : NULL,
    #if SID_SDK_CONFIG_ENABLE_LINK_TYPE_2 || SID_SDK_CONFIG_ENABLE_LINK_TYPE_3
            .sub_ghz_link_config = app_get_sub_ghz_config(),
    #endif
            .dev_ch = dev_ch,
        };

    struct sid_qa_pwr_meas_if pwr_meas_if = {
        .enter_func = pwr_meas_mode_enter,
        .exit_func = pwr_meas_mode_exit,
        .blocked_func = pwr_meas_is_blocked,
        .unblocked_func = pwr_meas_is_unblocked,
    };
    sid_qa_set_config(&config, &pwr_meas_if);
    TL_LOG_I("sid QA cli application started...");


}


void app_sidewalk_sch(void)
{

     sid_cli_process();
     sid_qa_process(QA_PROC_NO_WAIT);

}
