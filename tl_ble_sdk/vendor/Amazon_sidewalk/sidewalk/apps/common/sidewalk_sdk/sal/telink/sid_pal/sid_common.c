/********************************************************************************************************
 * @file    sid_common.c
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
#include "stack/ble/ble.h"
#include "sid_ble_adapter.h"
#include <sid_error.h>
#include <sid_error.h>
#include <sid_sdk_config.h>
#include <sid_pal_common_ifc.h>
#include <sid_pal_temperature_ifc.h>

#ifdef CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110
#include <lr11xx_gnss_wifi_config.h>
#endif /* CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 */


#if defined(CONFIG_SIDEWALK_SUBGHZ_RADIO_SX126X)
#include <sx126x_config.h>
static void set_radio_config(const void *cfg)
{
    set_radio_sx126x_device_config((const radio_sx126x_device_config_t *)cfg);
}
#elif defined(CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110)
#include <lr11xx_config.h>
static void set_radio_config(const void *cfg)
{
    set_radio_lr11xx_device_config((const radio_lr11xx_device_config_t *)cfg);
}
#endif /* CONFIG_SIDEWALK_SUBGHZ_RADIO */

sid_error_t sid_pal_common_init(const platform_specific_init_parameters_t *platform_init_parameters)
{
    if (!platform_init_parameters) {
        return SID_ERROR_INCOMPATIBLE_PARAMS;
    }

#if CONFIG_SIDEWALK_SUBGHZ_SUPPORT
    if (!platform_init_parameters->radio_cfg) {
        return SID_ERROR_INCOMPATIBLE_PARAMS;
    }
    set_radio_config(platform_init_parameters->radio_cfg);
#endif /* defined(CONFIG_SIDEWALK_SUBGHZ_SUPPORT) */

#if defined(CONFIG_SIDEWALK_TEMPERATURE)
    sid_error_t ret_code = sid_pal_temperature_init();
    if (ret_code) {
        TL_LOG_E("Sidewalk Init temperature pal  err: %d", ret_code);
    }
#endif

#ifdef CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110
    set_lr11xx_gnss_wifi_config(platform_init_parameters->gnss_wifi_cfg);
#endif /* CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110 */
    return SID_ERROR_NONE;
}

sid_error_t sid_pal_common_deinit(void)
{
#if defined(CONFIG_SIDEWALK_SUBGHZ_SUPPORT)
    sid_pal_radio_deinit();
#endif
    return SID_ERROR_NONE;
}
