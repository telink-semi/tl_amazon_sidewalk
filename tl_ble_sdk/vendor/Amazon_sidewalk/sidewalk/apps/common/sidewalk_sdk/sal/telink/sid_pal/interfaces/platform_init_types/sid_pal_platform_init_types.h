/*
 * Copyright 2023-2025 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_PLATFORM_INIT_TYPES_H
#define SID_PAL_PLATFORM_INIT_TYPES_H

#if SID_SDK_CONFIG_ENABLE_LINK_TYPE_2 || SID_SDK_CONFIG_ENABLE_LINK_TYPE_3
#if SID_RADIO_PLATFORM_LR11XX_EXP
#include <lr11xx_config.h>
#elif SID_RADIO_PLATFORM_LR1110
#include <lr1110_config.h>
#else
#include <sx126x_config.h>
#endif
#endif

#if defined(SID_SDK_CONFIG_ENABLE_LOCATION) && SID_SDK_CONFIG_ENABLE_LOCATION
#if defined(SID_RADIO_PLATFORM_LR11XX_EXP) && SID_RADIO_PLATFORM_LR11XX_EXP
#include <lr11xx_gnss_wifi_config.h>
#endif
#endif

typedef struct {
// place holder for platform specific init parameters
#if SID_SDK_CONFIG_ENABLE_LINK_TYPE_2 || SID_SDK_CONFIG_ENABLE_LINK_TYPE_3
#if SID_RADIO_PLATFORM_LR11XX_EXP
    radio_lr11xx_device_config_t *radio_cfg;
#elif SID_RADIO_PLATFORM_LR1110
    radio_lr1110_device_config_t *radio_cfg;
#else
    radio_sx126x_device_config_t *radio_cfg;
#endif
#endif

#if defined(SID_SDK_CONFIG_ENABLE_LOCATION) && SID_SDK_CONFIG_ENABLE_LOCATION
#if defined(SID_RADIO_PLATFORM_LR11XX_EXP) && SID_RADIO_PLATFORM_LR11XX_EXP
    lr11xx_gnss_wifi_config_t *gnss_wifi_cfg;
#endif
#endif
} platform_specific_init_parameters_t;

#endif
