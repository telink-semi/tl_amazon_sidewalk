/********************************************************************************************************
 * @file    sid_delay.c
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
/** @file sid_delay.c
 *  @brief Sidewalk delay implementation.
 */

#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "sid_ble_adapter.h"
#include <sid_pal_delay_ifc.h>
#if (FREERTOS_ENABLE)
    #include "tlk_riscv.h"
    #include <FreeRTOS.h>
    #include <task.h>
    #include <timers.h>
    #include <queue.h>
    #include <event_groups.h>
    #include "app_freertos.h"
#endif

void sid_pal_delay_us(uint32_t delay)
{
    delay_us(delay); //todo
}

void sid_pal_scheduler_delay_ms(uint32_t delay)
{
    return delay_ms(delay);
    #if (FREERTOS_ENABLE)
    if (delay == 0) return;
//    const TickType_t ticks = (delay + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS;
    const TickType_t ticks =  delay * portTICK_PERIOD_MS;
    vTaskDelay(ticks);
    #endif
}
