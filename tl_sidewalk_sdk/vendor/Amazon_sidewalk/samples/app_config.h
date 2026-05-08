/********************************************************************************************************
 * @file    app_config.h
 *
 * @brief   This is the header file for BLE SDK
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
#pragma once

#include "config.h"

#define CONN_MAX_NUM_CONFIG CONN_MAX_NUM_C0_P1
#define ACL_CENTRAL_MAX_NUM 0 // ACL central maximum number
#define ACL_PERIPHR_MAX_NUM 1 // ACL peripheral maximum number

///////////////////////// Feature Configuration////////////////////////////////////////////////
#define ACL_PERIPHR_SMP_ENABLE        0 //1 for smp,  0 no security
#define BLE_OTA_SERVER_ENABLE         1
#if AMAZON_DIAG_DEMO
#define BLE_APP_PM_ENABLE             0
#else
#define BLE_APP_PM_ENABLE             1
#endif

#if BLE_APP_PM_ENABLE
#define PM_DEEPSLEEP_RETENTION_ENABLE 1
#endif

#define BATT_CHECK_ENABLE             0

#ifndef AMAZON_DIAG_DEMO
    #define AMAZON_DIAG_DEMO 0
#endif

#ifndef AMAZON_DUT_DEMO
    #define AMAZON_DUT_DEMO 0
#endif

#ifndef AMAZON_900_DEMO
    #define AMAZON_900_DEMO 0
#endif
/* Flash Protection:
 * 1. Flash protection is enabled by default in SDK. User must enable this function on their final mass production application.
 * 2. User should use "Unlock" command in Telink BDT tool for Flash access during development and debugging phase.
 * 3. Flash protection demonstration in SDK is a reference design based on sample code. Considering that user's final application may
 *    different from sample code, for example, user's final firmware size is bigger, or user have a different OTA design, or user need
 *    store more data in some other area of Flash, all these differences imply that Flash protection reference design in SDK can not
 *    be directly used on user's mass production application without any change. User should refer to sample code, understand the
 *    principles and methods, then change and implement a more appropriate mechanism according to their application if needed.
 */
#define APP_FLASH_PROTECTION_ENABLE 1

#define AMAZON_SIDEWALK_OPTIMIZE_EN 0

#define  FLASH_4LINE_MODE_ENABLE 1
///////////////////////// OS settings /////////////////////////////////////////////////////////
#define config_MAIN_THREAD_METHOD 0
#define FREERTOS_ENABLE           1
#define OS_SEPARATE_STACK_SPACE   1 //Separate the task stack and interrupt stack space
#if config_MAIN_THREAD_METHOD
#define configTOTAL_HEAP_SIZE     ( 25 * 1024)
#else
#if AMAZON_DIAG_DEMO || AMAZON_DUT_DEMO
#define configTOTAL_HEAP_SIZE     ( 38 * 1024)
#else
#define configTOTAL_HEAP_SIZE     ( 36 * 1024)
#endif
#endif
#define configISR_PLIC_STACK_SIZE 640
#define configTIMER_TASK_STACK_DEPTH 512
#define config_HW_SELECT 0

/////////////////////// Board Select Configuration ///////////////////////////////
#if (MCU_CORE_TYPE == MCU_CORE_B91)
    #define BOARD_SELECT BOARD_951X_EVK_C1T213A20
#elif (MCU_CORE_TYPE == MCU_CORE_B92)
    #define BOARD_SELECT BOARD_952X_EVK_C1T266A20
#elif (MCU_CORE_TYPE == MCU_CORE_TL721X)
    #define BOARD_SELECT BOARD_721X_EVK_C1T315A20
#elif (MCU_CORE_TYPE == MCU_CORE_TL321X)
    #define BOARD_SELECT BOARD_321X_EVK_C1T335A20 //BOARD_321X_EVK_C1T331A20 //BOARD_321X_EVK_C1T335A20
#elif (MCU_CORE_TYPE == MCU_CORE_TL322X)
    #define BOARD_SELECT BOARD_322X_EVK_C1T371A20
#endif

///////////////////////// UI Configuration ////////////////////////////////////////////////////
#if AMAZON_900_DEMO

#if config_HW_SELECT
#define UI_LED_ENABLE      1
#define UI_KEYBOARD_ENABLE 1
#else
#define UI_LED_ENABLE      0
#define UI_KEYBOARD_ENABLE 1
#endif
#else
#define UI_LED_ENABLE      0
#define UI_KEYBOARD_ENABLE 0
#define UI_BUTTON_ENABLE   0
#endif

///////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
#define DEBUG_GPIO_ENABLE    0

#define TLKAPI_RTT_PRINT     0

#define TLKAPI_DEBUG_ENABLE  1
#define TLKAPI_DEBUG_CHANNEL TLKAPI_DEBUG_CHANNEL_GSUART
#if AMAZON_DIAG_DEMO || AMAZON_DUT_DEMO
#define TLKAPI_DEBUG_FIFO_NUM  16
#endif

#if AMAZON_DIAG_DEMO || AMAZON_DUT_DEMO
#define TLKAPI_DEBUG_GPIO_PIN GPIO_PA0
#else
#define TLKAPI_DEBUG_GPIO_PIN GPIO_PA1
#endif

#define APP_LOG_EN           1
#define APP_CONTR_EVT_LOG_EN 0 //controller event
#define APP_HOST_EVT_LOG_EN  1
#define APP_KEY_LOG_EN       1
#define APP_BUTTON_LOG_EN    1

#define JTAG_DEBUG_ENABLE    0 //if use JTAG, change this


/////////////////// DEEP SAVE FLG //////////////////////////////////
#define USED_DEEP_ANA_REG PM_ANA_REG_POWER_ON_CLR_BUF1 //u8,can save 8 bit info when deep
#define LOW_BATT_FLG      BIT(0)                       //if 1: low battery
#define CONN_DEEP_FLG     BIT(1)                       //if 1: conn deep, 0: adv deep


#if FREERTOS_ENABLE
/////////////////////////////////////// PRINT DEBUG INFO ///////////////////////////////////////
//    #undef UI_KEYBOARD_ENABLE
//    #define UI_KEYBOARD_ENABLE           1


    #define traceAPP_LED_Task_Toggle()   //gpio_toggle(GPIO_CH01);
    #define traceAPP_BLE_Task_BEGIN()    //gpio_write(GPIO_CH02,1);
    #define traceAPP_BLE_Task_END()      //gpio_write(GPIO_CH02,0);
    #define traceAPP_KEY_Task_BEGIN()    //gpio_write(GPIO_CH03,1);
    #define traceAPP_KEY_Task_END()      //gpio_write(GPIO_CH03,0);
    #define traceAPP_BAT_Task_BEGIN()    //gpio_write(GPIO_CH04,1);
    #define traceAPP_BAT_Task_END()      //gpio_write(GPIO_CH04,0);

    #define traceAPP_MUTEX_Task_BEGIN()  //gpio_write(GPIO_CH05,1);
    #define traceAPP_MUTEX_Task_END()    //gpio_write(GPIO_CH05,0);

    #define tracePort_IrqHandler_BEGIN() //gpio_write(GPIO_CH06,1);
    #define tracePort_IrqHandler_END()   //gpio_write(GPIO_CH06,0);

#endif

#if CONFIG_SIDEWALK_SUBGHZ_SUPPORT
#define  CONFIG_SIDEWALK_SUBGHZ_RADIO_SX126X  1
#endif
#define TIMER_SOURCE_32K 1
#define TIMER_SOURCE_OS  2
#define CONFIG_DIO3_FOR_ANT_SW 1
#define CONFIG_TIMER_SOURCE  TIMER_SOURCE_32K


#define CFG_BLE_PERIPHERAL_PREF_LATENCY 0
#define CFG_BLE_PERIPHERAL_PREF_TIMEOUT 400
#define CFG_SIDEWALK_BLE_ADV_INT_SLOW 1000   //ms
#define CFG_SIDEWALK_BLE_ADV_INT_FAST 160     //ms
#define CFG_SIDEWALK_BLE_ADV_INT_PRECISION 0 //ms
#define CFG_SIDEWALK_BLE_ADV_INT_TRANSITION    3000  //10ms

#define CONFIG_SIDEWALK_BLE_NAME    "Telink_SideWalk"


#define CONFIG_SIDEWALK_LOG_MSG_LENGTH_MAX 128

#define CONFIG_SIDEWALK_SWI_PRIORITY 6
#define CONFIG_SIDEWALK_THREAD_PRIORITY 5

#define CONFIG_SID_END_DEVICE_ECHO_MSGS 0
#define CONFIG_SIDEWALK_THREAD_QUEUE_TIMEOUT_VALUE 0
#define GPIO_COUNT 2

#define CONFIG_SIDEWALK_MFG_PARSER_MAX_ELEMENT_SIZE 256

#define  CONFIG_SID_END_DEVICE_AUTO_CONN_REQ 1

#define  CONFIG_SIDEWALK_THREAD_STACK_SIZE 1024
#define  CONFIG_SIDEWALK_THREAD_QUEUE_SIZE 16

#define CFG_ADR_SIDEWALK_MFG_2M_FLASH 0x1F5000 //4k
#define CFG_ADR_SIDEWALK_2M_FLASH  0x1F6000   //8k 0x1F1000
#define CFG_ADR_SIDEWALK_2M_TRIM_FLASH  0x1FE500

#define CFG_ADR_SIDEWALK_MFG_1M_FLASH 0xF5000  //4k
#define CFG_ADR_SIDEWALK_1M_FLASH  0xF6000   //8k 0x1F1000
#define CFG_ADR_SIDEWALK_1M_TRIM_FLASH  0xFE500

#define CONFIG_SIDEWALK_SID_SUBG_TRIM_VAL 0x1212

//#define SID_BUILD_DEBUG 1


#define CONFIG_LOG
#define CONFIG_LOG_LEVEL_INFO 0
#define CONFIG_LOG_LEVEL_ERROR 1
#define CONFIG_LOG_LEVEL_DEBUG 0
#define CONFIG_LOG_LEVEL_WRN 1

//#define  CONFIG_SIDEWALK_THREAD_TIMER 1
//#define  CONFIG_SIDEWALK_MFG_STORAGE_DIAGNOSTIC 1
#define  OS_COMPILE_OPTIMIZE_EN 1
#define LL_FEATURE_SUPPORT_LE_2M_PHY 0
#define LL_FEATURE_SUPPORT_LE_CODED_PHY 0
#define LL_FEATURE_SUPPORT_LE_PAST_SENDER 0
#define LL_FEATURE_SUPPORT_LE_PAST_RECIPIENT 0
#define LL_FEATURE_SUPPORT_LE_EXTENDED_ADVERTISING 0
#define LL_FEATURE_SUPPORT_LE_DECISION_BASED_ADVERTISING_FILTERING  0
#define LL_FEATURE_SUPPORT_LE_LEGACY_SCANNING 0
#define LL_FEATURE_SUPPORT_LE_EXTENDED_SCANNING 0
#define LL_FEATURE_SUPPORT_LE_EXTENDED_INITIATE 0
#define LL_FEATURE_SUPPORT_LE_PERIODIC_ADVERTISING 0
#define LL_FEATURE_SUPPORT_LE_PERIODIC_ADVERTISING_SYNC 0
#define LL_FEATURE_SUPPORT_CHANNEL_SELECTION_ALGORITHM2 0

#if config_HW_SELECT
#define RADIO_MOSI       GPIO_PB7
#define RADIO_MISO       GPIO_PB6
#define RADIO_SCLK       GPIO_PB5
#define RADIO_NSS        GPIO_PB4

#define RADIO_RESET      GPIO_PE0
#define RADIO_BUSY       GPIO_PE1
#define RADIO_DIO_1      GPIO_PE2
#define ANT_SWITCH_POWER GPIO_PE3
#else
#define RADIO_MOSI       GPIO_PE4
#define RADIO_MISO       GPIO_PE3
#define RADIO_SCLK       GPIO_PE0
#define RADIO_NSS        GPIO_PE1
//#define RADIO_NSS        GPIO_PC2

#define RADIO_RESET      GPIO_PB5
#define RADIO_BUSY       GPIO_PB4
#define RADIO_DIO_1      GPIO_PD3
#define ANT_SWITCH_POWER GPIO_PE2
#endif

#include "../common/default_config.h"
