/********************************************************************************************************
 * @file
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
#include "stack/ble/ble.h"

#include <sid_app_sbdt_demo.h>
#include <sid_sdk_version.h>

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
#include <sid_utils.h>

#include <app_mfg_config.h>
#include "app_buffer.h"
#include "app_ui.h"
#include "app_mem.h"

#include "sid_demo_parser.h"
#include "sid_demo_types.h"

#define KEY1  0x01
#define KEY2  0x02
#define KEY3  0xf1
#define KEY4  0xf0

#define PARAM_UNUSED (0U)

#define MAIN_TASK_STACK_SIZE        (4096 / sizeof(configSTACK_DEPTH_TYPE))

#define APP_MAX_WRITE_SIZE 256     //shoubld be 16* ,max 240

int blt_ota_save_data(u32 flash_addr, int len, u8 *data);
int blt_ota_crc_check_whole(int size);
void blt_ota_writeBootMark(void);
void app_sbdt_transfer_init(struct sid_handle *handle);

_attribute_ble_data_retention_  static app_context_t g_stru_app_context ;
static uint32_t sbdt_file_size = 0;
static bool sbdt_file_crc_check_rt = true;
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


static int32_t init_and_start_link(app_context_t *context, struct sid_config *config, uint32_t link_mask)
{
    if (config->link_mask != link_mask) {
        sid_error_t ret = SID_ERROR_NONE;
        if (context->sidewalk_handle != NULL) {
            ret = sid_deinit(context->sidewalk_handle);
            if (ret != SID_ERROR_NONE) {
                TL_LOG_E("failed to deinitialize sidewalk, link_mask:%x, err:%d");
                goto error;
            }
        }
        struct sid_handle *sid_handle = NULL;
        config->link_mask = link_mask;
        // Initialise sidewalk
        ret = sid_init(config, &sid_handle);
        if (ret != SID_ERROR_NONE) {
            TL_LOG_E("failed to initialize sidewalk link_mask:%x, err:%d", link_mask, (int)ret);
            goto error;
        }
        // Register sidewalk handler to the application context
         context->sidewalk_handle = sid_handle;
        TL_LOG_D("demo_sbdt_app_init start");
        demo_sbdt_app_init(context);
        app_sbdt_transfer_init(sid_handle);
        TL_LOG_D("demo_sbdt_app_init done");
        // Start the sidewalk stack
        ret = sid_start(sid_handle, link_mask);
        if (ret != SID_ERROR_NONE) {
            TL_LOG_E("failed to start sidewalk, link_mask:%x, err:%d", link_mask, (int)ret);
            goto error;
        }
    }
    return 0;

error:
    context->sidewalk_handle = NULL;
    config->link_mask = 0;
    return -1;
}

void app_sbdt_transfer_init(struct sid_handle *handle)
{
    TL_LOG_I("sid_bulk_data_transfer_init called");

    sid_error_t ret = sid_bulk_data_transfer_init(
        &(struct sid_bulk_data_transfer_config){ .callbacks = demo_sbdt_app_get_sid_event_callbacks()}, handle);
    if (ret != SID_ERROR_NONE) {
        TL_LOG_E("sid_bulk_data_transfer_init returned %d",ret);
    }
    TL_LOG_I("blt_ota_init %x",blc_ota_getNextFirmwareStartAddress());
}
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
void app_radio_event_notify(sid_pal_radio_events_t evt)
{

}

void app_radio_dio_irq_handler(void)
{
    return;
}
#endif


#define APP_BIN_SIZE 500*1024


void app_platform_reset(void)
{
    sys_reboot();
}

void app_platform_flash_init(void)
{
    sbdt_file_crc_check_rt = true;
}

uint32_t app_platform_flash_get_free_space(void)
{
   return APP_BIN_SIZE;
}


void app_sid_sbdt_start(int file_size)
{
    sbdt_file_size = file_size;
    blt_ota_crc_init(sbdt_file_size);
}


bool app_platform_flash_write(uint32_t offset, void *data, size_t size)
{

    #define FLASH_ERASE_SIZE  0x1000
    #define FLASH_ERASE_MASK  (FLASH_ERASE_SIZE - 1)
    #define FLASH_SECTOR_MASK (0xFFFFF000)
    static u8 fw_flag = 0;
    int ret = 0;

    TL_LOG_D("app_platform_flash_write %x %x %d\r\n",offset,Addr,size);

    if(fw_flag == 0)
    {
        tlkapi_send_string_data(APP_LOG_EN, "fw updata ", data, 16);
        fw_flag = 1;
    }
    int total_segments = (size + APP_MAX_WRITE_SIZE - 1) / APP_MAX_WRITE_SIZE;
    for (int i = 0; i < total_segments; i++)
    {
        int start = i * APP_MAX_WRITE_SIZE;
        int remain_len = size - start;
        int curr_len = (remain_len > APP_MAX_WRITE_SIZE) ? APP_MAX_WRITE_SIZE : remain_len;
        if(((start + offset) & FLASH_ERASE_MASK ) ==  0)
        {
            flash_erase_sector(blc_ota_getNextFirmwareStartAddress() + ((start + offset) & FLASH_SECTOR_MASK));
        }
        else if( ((start + offset+curr_len) & FLASH_SECTOR_MASK )!= ((start + offset) & FLASH_SECTOR_MASK ))
        {
            flash_erase_sector(blc_ota_getNextFirmwareStartAddress() + ((start + offset + curr_len) & FLASH_SECTOR_MASK));
        }

        ret = blt_ota_save_data(start + offset,curr_len,((uint8_t *)data)+ start);
        if(ret != 0)
        {
           sbdt_file_crc_check_rt = false;
           TL_LOG_E("ota_save_data fail %d !!!!", ret);
           fflush(NULL);
           return false;
       }
    }
    TL_LOG_D("app_platform_flash_write done\r\n");

    return true;
}

bool app_platform_flash_check(uint32_t size, uint32_t expected_crc)
{
    ((void)size);
    ((void)expected_crc);
    if(0 != blt_ota_crc_check_whole(sbdt_file_size))
    {
        sbdt_file_crc_check_rt = false;
        TL_LOG_E("crc fail %d !!!!");
        return false;
    }
    return sbdt_file_crc_check_rt;
}

bool app_platform_flash_finalize(uint32_t size, uint32_t crc)
{
    ((void)size);
    ((void)crc);
    fflush(NULL);
    TL_LOG_I("finalize %d %d %d\r\n",sbdt_file_size,size,sbdt_file_crc_check_rt);
    if(sbdt_file_crc_check_rt)
    {
        #if (APP_FLASH_PROTECTION_ENABLE)
        app_flash_protection_operation(FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_BEGIN, 0, 0);
        #endif
        blt_ota_writeBootMark();
        #if (APP_FLASH_PROTECTION_ENABLE)
        app_flash_protection_operation(FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_END, 0, 0);
        #endif
    }
    demo_sys_reboot();
    return sbdt_file_crc_check_rt;
}


void app_env_init(void)
{
    g_stru_app_context.hw_ifc.platform_reset = app_platform_reset;
    g_stru_app_context.hw_ifc.platform_flash_init = app_platform_flash_init;
    g_stru_app_context.hw_ifc.platform_flash_get_free_space = app_platform_flash_get_free_space;
    g_stru_app_context.hw_ifc.platform_flash_write = app_platform_flash_write;
    g_stru_app_context.hw_ifc.platform_flash_check = app_platform_flash_check;
    g_stru_app_context.hw_ifc.platform_flash_finalize = app_platform_flash_finalize;
}

static void main_thread(void *context)
{
    app_context_t *app_context = (app_context_t *)context;
    struct sid_end_device_characteristics dev_ch = {
        .type = SID_END_DEVICE_TYPE_STATIC,
        .power_type = SID_END_DEVICE_POWERED_BY_BATTERY_AND_LINE_POWER,
        .qualification_id = 0x0005,
    };

    struct sid_config config = {
        .link_mask = 0,
        .dev_ch = dev_ch,
        .callbacks = demo_app_get_sid_event_callbacks(),
        .link_config = app_get_ble_config(),
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
        .sub_ghz_link_config = app_get_sub_ghz_config(),
#endif
    };

    if (init_and_start_link(app_context, &config, SID_LINK_TYPE_1) != 0) {
            goto error;
    }
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
    sid_pal_radio_rx_packet_t         g_rx;
    sid_pal_radio_init(app_radio_event_notify, app_radio_dio_irq_handler, &(g_rx));
    if(0 !=sid_pal_radio_sleep(UINT32_MAX)) //save power
    {
        TL_LOG_E("sid_pal_radio_sleep fail");
    }
#endif

    while (1) {
        struct app_demo_event  event;
        if (xQueueReceive(app_context->event_queue, &event, portMAX_DELAY) == pdTRUE) {
            TL_LOG_D("event %d\r\n",event.type);
            demo_app_main_task_event_handler(&event);
        }
    }

error:
    if (app_context->sidewalk_handle != NULL) {
        sid_stop(app_context->sidewalk_handle, SID_LINK_TYPE_1);
        sid_deinit(app_context->sidewalk_handle);
        app_context->sidewalk_handle = NULL;
    }

    vTaskDelete(NULL);
}



void Portble_btn_press(u8 key)
{
    if(KEY1 == key)
    {
        demo_app_button_event_handler(0);
    }
    else if(KEY2 == key)
    {
        demo_app_button_event_handler(1);
    }
    else    if(KEY3 == key)
    {
        demo_app_button_event_handler(2);
    }
    else
    {
        demo_app_button_event_handler(3);
    }
}

void Portble_btn_d_press(u8 key)
{
    ARG_UNUSED(key);
    demo_app_button_event_handler(0);
}
void Portble_btn_l_press(u8 key)
{
    ARG_UNUSED(key);
}

int app_start(void)
{
    app_env_init();
    #if BLE_APP_PM_ENABLE
    void app_sleep_config(void);
    app_sleep_config();
    #endif
    platform_parameters_t platform_parameters = {
            .mfg_store_region.addr_start = sid_mfg_get_start_addr(),
            .mfg_store_region.addr_end = sid_mfg_get_end_addr(),
#ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
            .platform_init_parameters.radio_cfg = (radio_sx126x_device_config_t*)get_radio_cfg(),
#endif
    };

    sid_error_t ret_code = sid_platform_init(&platform_parameters);
    if (ret_code != SID_ERROR_NONE) {
        TL_LOG_E("Sidewalk Platform Init err: %d", ret_code);
         configASSERT(0);
    }
    TL_LOG_D("Sidewalk Platform Init done2");
    g_stru_app_context.event_queue = NULL;
    g_stru_app_context.main_task = NULL;
    g_stru_app_context.sidewalk_handle = NULL;
    if (pdPASS != xTaskCreate(main_thread, "sidewalk", MAIN_TASK_STACK_SIZE, &g_stru_app_context, CONFIG_SIDEWALK_THREAD_PRIORITY, &g_stru_app_context.main_task)) {
        TL_LOG_E("sidewalk xTaskCreate init  err");
         configASSERT(0);
    }
    return 0;
}

#if BLE_APP_PM_ENABLE
extern void app_sid_subg_sleep_enter(u8 e, u8 *p, int n);
extern void app_sid_subg_wakeup(u8 e, u8 *p, int n);
extern void proc_keyboard(u8 e, u8 *p, int n);
extern void proc_keyboardSupend (u8 e, u8 *p, int n);


_attribute_ram_code_ void app_sid_sleep_enter(u8 e, u8 *p, int n)
{
    (void)e;
    (void)p;
    (void)n;
    #if (UI_KEYBOARD_ENABLE || UI_BUTTON_ENABLE)
    app_set_kb_wakeup(e,p,n);
    #endif
    #ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
    app_sid_subg_sleep_enter(e,p,n);
    #endif
}

void app_sid_wakeup(u8 e, u8 *p, int n)
{
    #if (UI_KEYBOARD_ENABLE || UI_BUTTON_ENABLE)
    #if (FREERTOS_ENABLE)
    proc_keyboardSupend(e,p,n);
    #else
    proc_keyboard(e,p,n);
    #endif
    #endif
    #ifdef CONFIG_SIDEWALK_SUBGHZ_SUPPORT
    //app_sid_subg_wakeup(e,p,n);
    #endif
}

void app_sleep_config(void)
{
    blc_ll_registerTelinkControllerEventCallback(BLT_EV_FLAG_SLEEP_ENTER, &app_sid_sleep_enter);
    blc_ll_registerTelinkControllerEventCallback(BLT_EV_FLAG_GPIO_EARLY_WAKEUP, &app_sid_wakeup);
    blc_ll_registerTelinkControllerEventCallback(BLT_EV_FLAG_SUSPEND_EXIT, &app_sid_subg_wakeup);
}

#endif

