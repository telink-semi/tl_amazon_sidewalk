/********************************************************************************************************
 * @file    app_mfg_config.h
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
#include <stdbool.h>
#include <sid_pal_mfg_store_ifc.h>



//#define APP_MFG_CFG_FLASH_START CFG_ADR_SIDEWALK_MFG_2M_FLASH



#define INVALID_VERSION (0xFFFFFFFF)

static inline bool app_mfg_cfg_is_empty(void)
{
    return sid_pal_mfg_store_get_version() == INVALID_VERSION;
}
uint32_t sid_mfg_get_start_addr(void);
uint32_t sid_mfg_get_end_addr(void);
uint32_t sid_mfg_set_start_addr(uint32_t  addr);
