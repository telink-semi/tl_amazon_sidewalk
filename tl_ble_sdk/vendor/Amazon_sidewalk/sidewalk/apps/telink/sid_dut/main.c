/********************************************************************************************************
 * @file    main.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2022
 *
 * @par     Copyright (c) 2022, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "stack/ble/ble.h"
#include "app.h"

#if (FREERTOS_ENABLE)
    #include "tlk_riscv.h"
    #include <FreeRTOS.h>
    #include <task.h>
    #include "app_freertos.h"
#endif

#if TLKAPI_RTT_PRINT
#include "RTT/SEGGER_RTT.h"
#endif
/**
 * @brief       BLE RF interrupt handler.
 * @param[in]   none
 * @return      none
 */
_attribute_ram_code_ void rf_irq_handler(void)
{
    DBG_CHN14_HIGH;

    blc_sdk_irq_handler();

    DBG_CHN14_LOW;
}
#if (FREERTOS_ENABLE)
PLIC_ISR_REGISTER_OS(rf_irq_handler, IRQ_ZB_RT)
#else
PLIC_ISR_REGISTER(rf_irq_handler, IRQ_ZB_RT)
#endif
/**
 * @brief       System timer interrupt handler.
 * @param[in]   none
 * @return      none
 */
_attribute_ram_code_ void stimer_irq_handler(void)
{
    DBG_CHN15_HIGH;
    blc_sdk_irq_handler();
    DBG_CHN15_LOW;
}
#if (FREERTOS_ENABLE)
PLIC_ISR_REGISTER_OS(stimer_irq_handler, IRQ_SYSTIMER)
#else
PLIC_ISR_REGISTER(stimer_irq_handler, IRQ_SYSTIMER)
#endif
void gpio_irq1_handler(void);
volatile unsigned int gpio_irq0_cnt = 0;
_attribute_ram_code_sec_ void gpio_irq0_handler(void)
{
    gpio_irq0_cnt++;
    sid_pal_radio_irq_process();
    gpio_clr_irq_status(GPIO_IRQ_IRQ0);
}
#if (FREERTOS_ENABLE)
PLIC_ISR_REGISTER_OS(gpio_irq1_handler, IRQ_GPIO_IRQ0);
#else
PLIC_ISR_REGISTER(gpio_irq1_handler, IRQ_GPIO_IRQ0);
#endif

/**
 * @brief      application system initialization
 * @param[in]  none.
 * @return     none.
 */
__INLINE void blc_app_system_init(void)
{
#if (MCU_CORE_TYPE == MCU_CORE_B91)
    sys_init(DCDC_1P4_LDO_1P8, VBAT_MAX_VALUE_GREATER_THAN_3V6, INTERNAL_CAP_XTAL24M);
    gpio_set_up_down_res(GPIO_SWS, GPIO_PIN_PULLUP_1M);
    wd_stop();
    CCLK_32M_HCLK_32M_PCLK_16M;
#elif (MCU_CORE_TYPE == MCU_CORE_B92)
    sys_init(DCDC_1P4_LDO_2P0, VBAT_MAX_VALUE_GREATER_THAN_3V6, GPIO_VOLTAGE_3V3, INTERNAL_CAP_XTAL24M);
    pm_update_status_info(1);
    gpio_set_up_down_res(GPIO_SWS, GPIO_PIN_PULLUP_1M);
    wd_32k_stop();
    CCLK_32M_HCLK_32M_PCLK_16M;
#elif (MCU_CORE_TYPE == MCU_CORE_TL721X)
    sys_init(DCDC_0P94_DCDC_1P8, VBAT_MAX_VALUE_GREATER_THAN_3V6, INTERNAL_CAP_XTAL24M);
    pm_update_status_info(1);
    gpio_set_up_down_res(GPIO_SWS, GPIO_PIN_PULLUP_1M);
    wd_32k_stop();
    wd_stop();
    PLL_240M_CCLK_48M_HCLK_48M_PCLK_48M_MSPI_48M;
#elif (MCU_CORE_TYPE == MCU_CORE_TL321X)
    sys_init(DCDC_1P25_LDO_1P8, VBAT_MAX_VALUE_GREATER_THAN_3V6, INTERNAL_CAP_XTAL24M);
    pm_update_status_info(1);
    gpio_set_up_down_res(GPIO_SWS, GPIO_PIN_PULLUP_1M);
    wd_32k_stop();
    wd_stop();
    PLL_192M_CCLK_48M_HCLK_48M_PCLK_48M_MSPI_48M;
#elif (MCU_CORE_TYPE == MCU_CORE_TL322X)
    sys_init(DCDC_1P25_LDO_1P8, VBAT_MAX_VALUE_GREATER_THAN_3V6, INTERNAL_CAP_XTAL24M);
    pm_update_status_info(1);
    gpio_set_up_down_res(GPIO_SWS, GPIO_PIN_PULLUP_1M);
    wd_32k_stop();
    wd_stop();
    PLL_192M_D25F_64M_HCLK_N22_32M_PCLK_32M_MSPI_48M;
    
    #if !defined(TLK_ONLY_BLE_HOST)
        pm_set_dig_module_power_switch(FLD_PD_ZB_EN, PM_POWER_UP); //Temporarily placed, wait for the driver to confirm afterwards
        rf_n22_dig_init();
    #endif
#elif (MCU_CORE_TYPE == MCU_CORE_TL323X)
    sys_init(DCDC_1P25_LDO_1P8, VBAT_MAX_VALUE_GREATER_THAN_3V6, INTERNAL_CAP_XTAL24M);
    gpio_set_up_down_res(GPIO_SWS, GPIO_PIN_PULLUP_1M);
    wd_32k_stop();
    wd_stop();
    PLL_192M_CCLK_96M_HCLK_48M_PCLK_48M_MSPI_48M;
    //PLL_192M_CCLK_48M_HCLK_48M_PCLK_48M_MSPI_48M;
#else
    #error "Not Supported Chip!!!"
#endif
}
void app_start(void);
int app_sidewalk_init(void);
/**
 * @brief       This is main function
 * @param[in]   none
 * @return      none
 */
_attribute_ram_code_ int main(void)
{
    DBG_CHN0_LOW;
    blc_ota_setFirmwareSizeAndBootAddress(410,MULTI_BOOT_ADDR_0x80000);
    /* this function must called before "sys_init()" when:
     * (1). For all IC: using 32K RC for power management,
       (2). For B91 only: even no power management */
//#if BLE_APP_PM_ENABLE
  // blc_pm_select_internal_32k_crystal();
//#endif
    blc_pm_select_external_32k_crystal();
    blc_app_system_init();

    /* detect if MCU is wake_up from deep retention mode */
    int deepRetWakeUp = pm_is_MCU_deepRetentionWakeup(); //MCU deep retention wakeUp

    #if !defined(TLK_ONLY_BLE_HOST)
    /* Place the RF in the N22 initialization. */
    rf_drv_ble_init();
    #endif

    gpio_init(!deepRetWakeUp);

    #if defined(TLK_ONLY_BLE_HOST)
    if (deepRetWakeUp) {
        sys_n22_init(N22_IRAM_STARTUP_ADDR);
    } else {
        sys_n22_init(N22_FW_DOWNLOAD_FLASH_ADDR);
    }
        sys_n22_start();
        tlk_mailbox_service_init();
        tlk_share_memory_service_init();
    #endif
    if (deepRetWakeUp) { //MCU wake_up from deepSleep retention mode
#if (FREERTOS_ENABLE)
        extern void vPortRestoreTick(void);
        vPortRestoreTick();
#endif
        user_init_deepRetn();
        bool sid_ble_is_enable(void);
        if(!sid_ble_is_enable())
        {
            int32_t sid_pal_radio_reinit(void);
            sid_pal_radio_reinit();
        }

        void app_uart_init(void);
        app_uart_init();

    } else { //MCU power_on or wake_up from deepSleep mode
        user_init_normal();
     
#if (FREERTOS_ENABLE)
        app_start();
#else
        app_sidewalk_init();
#endif
    }
    #if (JTAG_DEBUG_ENABLE || TLKAPI_RTT_PRINT)
    jtag_set_pin_en();
    #endif
    #if TLKAPI_RTT_PRINT
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
    SEGGER_RTT_WriteString(0, "###### Testing SEGGER_printf() ######\r\n");
    #endif
    irq_enable();

#if (FREERTOS_ENABLE)
    app_TaskCreate();

    vTaskStartScheduler();
    while (1);
#else

    while (1) {
        main_loop();
    }
#endif
    return 0;
}
