/********************************************************************************************************
 * @file    sid_critical_region.c
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
/** @file sid_critical_region.c
 *  @brief Critical region interface implementation.
 */
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "sid_ble_adapter.h"
#include <sid_pal_critical_region_ifc.h>
#include <assert.h>
#include "FreeRTOS.h"
#if (FREERTOS_ENABLE)
    #include "tlk_riscv.h"
    #include <FreeRTOS.h>
    #include <task.h>
    #include <timers.h>
    #include <queue.h>
    #include <event_groups.h>
    #include "app_freertos.h"
#endif

_attribute_ble_data_retention_ static volatile uint32_t  le_crit_cnt = 0;
_attribute_ble_data_retention_ static volatile uint32_t  saved_int_stat = 0;


#if (FREERTOS_ENABLE)


_attribute_ram_code_ void sid_pal_enter_critical_region(void)
{
    uint32_t curr_stat = portSET_INTERRUPT_MASK_FROM_ISR();
    if(le_crit_cnt == 0) {
        saved_int_stat = curr_stat;
    }
    le_crit_cnt++;
}


_attribute_ram_code_ void sid_pal_exit_critical_region(void)
{
    if (le_crit_cnt == 0) return;
    le_crit_cnt--;
    if(le_crit_cnt == 0) {
        portCLEAR_INTERRUPT_MASK_FROM_ISR( saved_int_stat );
    }
}

#else

static inline void compiler_barrier(void)
{
    //__asm volatile("" ::: "memory");
}


_attribute_ram_code_ void sid_pal_enter_critical_region(void)
{
    compiler_barrier();
    saved_int_stat = irq_disable();
    le_crit_cnt ++;
    compiler_barrier();
}


_attribute_ram_code_ void sid_pal_exit_critical_region(void)
{
    compiler_barrier();
    if (le_crit_cnt == 0) return;
    le_crit_cnt--;
    if (le_crit_cnt == 0) {
        irq_restore(saved_int_stat);
    }
    compiler_barrier();
}

#endif
