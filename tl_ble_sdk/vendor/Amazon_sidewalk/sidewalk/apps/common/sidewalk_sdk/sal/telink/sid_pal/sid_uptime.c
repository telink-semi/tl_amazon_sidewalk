/********************************************************************************************************
 * @file    sid_uptime.c
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
/** @file sid_uptime.c
 *  @brief Uptime interface implementation.
 */
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "sid_ble_adapter.h"
#if (FREERTOS_ENABLE)
    #include "tlk_riscv.h"
    #include <FreeRTOS.h>
    #include <task.h>
    #include <timers.h>
    #include <queue.h>
    #include <semphr.h>
    #include <event_groups.h>
    #include "app_freertos.h"
#endif
#include <sid_pal_uptime_ifc.h>

#define TIMER_RTC_MAX_PPM_TO_COMPENSATE 200

_attribute_data_retention_  uint32_t g_last_32k_counter = 0;
_attribute_data_retention_  uint32_t g_wrapped_tv_sec = 0;
_attribute_data_retention_ uint64_t sec_per_rollover_rc = (1ULL << 32) / 32000;  //
_attribute_data_retention_ uint64_t sec_per_rollover_xtal = (1ULL << 32) / 32768;  // 131072
_attribute_ram_code_ sid_error_t sid_pal_uptime_now(struct sid_timespec *result)
{
    if (!result) {
        return SID_ERROR_NULL_POINTER;
    }

#if (CONFIG_TIMER_SOURCE == TIMER_SOURCE_OS)

    uint64_t uptime_tick = xTaskGetTickCount();

    result->tv_sec = (sid_time_t)(uptime_tick / configTICK_RATE_HZ );

    result->tv_nsec = (uint32_t)(uptime_tick -result->tv_sec  *configTICK_RATE_HZ)*1000/configTICK_RATE_HZ *1000000;

#else
    sid_pal_enter_critical_region();
    uint32_t uptime_tick = clock_get_32k_tick();
    uint32_t miss_tick = 0;
    if(g_last_32k_counter > uptime_tick)  // overflow
    {
         if (g_clk_32k_src == CLK_32K_RC)
         {
             g_wrapped_tv_sec += sec_per_rollover_rc;
             miss_tick = 23296;
         }
         else
         {
             g_wrapped_tv_sec += sec_per_rollover_xtal;
         }
    }
    g_last_32k_counter = uptime_tick;
        //32k
    if (g_clk_32k_src == CLK_32K_RC)
    {
        result->tv_sec = (sid_time_t)(uptime_tick/32000 ) ;
        result->tv_nsec = (uptime_tick - result->tv_sec * 32000 + miss_tick )*31250 ;
        result->tv_sec += g_wrapped_tv_sec;
    }
    else
    {
        result->tv_sec = (sid_time_t)(uptime_tick/32768) ;
        uint32_t sub = uptime_tick & 0x7FFFUL;
        result->tv_nsec = sub*30517 + (sub*37/64);
        result->tv_sec += g_wrapped_tv_sec;
    }
    sid_pal_exit_critical_region();
#endif
//  TL_LOG_D("sid_pal_uptime_now: %d %d %d \n",uptime_tick,result->tv_sec,result->tv_nsec);

    return SID_ERROR_NONE;
}

void sid_pal_uptime_set_xtal_ppm(int16_t ppm)
{
    //todo

}

int16_t sid_pal_uptime_get_xtal_ppm(void)
{
    return TIMER_RTC_MAX_PPM_TO_COMPENSATE;
}
