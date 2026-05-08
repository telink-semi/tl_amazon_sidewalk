/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "app_ble_config.h"

#define countof(array_) \
    (1 \
        ? sizeof(array_)/sizeof((array_)[0]) \
        : sizeof(struct { int do_not_use_countof_for_pointers : ((void*)(array_) == (void*)&array_);}) \
        )

static const sid_ble_cfg_service_t ble_service = {
    .type = AMA_SERVICE,
    .id = {
        .type = UUID_TYPE_16,
        .uu = { 0xFE, 0x03 },
    },
};

static const sid_ble_cfg_descriptor_t ble_desc[] = {
    {
        .id = {
            .type = UUID_TYPE_16,
            .uu = { 0x29, 0x02 },
        },
        .perm = {
            .is_write = true,
        },
    }
};

static const sid_ble_cfg_characteristics_t ble_characteristics[] = {
    {
        .id = {
            .type = UUID_TYPE_128,
            .uu = { 0x3C, 0xC5, 0x61, 0xAB, 0x27, 0x04, 0x32, 0x92,
                        0x58, 0x4D, 0x6C, 0x7D, 0xC9, 0x96, 0xF9, 0x74 },
        },
        .properties = {
            .is_write_no_resp = true,
        },
        .perm = {
            .is_write = true,
        },
    },
    {
        .id = {
            .type = UUID_TYPE_128,
            .uu = { 0xFE, 0xD2, 0xF0, 0xE7, 0xB7, 0x53, 0x15, 0x90,
                        0xC1, 0x47, 0xCE, 0xFE, 0xC0, 0x83, 0x2E, 0xB3 },
        },
        .properties = {
            .is_notify = true,
        },
        .perm = {
            .is_none = true,
        },
    },
};

static const sid_ble_cfg_adv_param_t adv_param = {
    .type = AMA_SERVICE,
    .fast_enabled = true,
    .slow_enabled = true,
    .fast_interval = 256,
    .fast_timeout = 3000,
    .slow_interval = 1600,
    .slow_timeout = 0,
};
static const sid_ble_cfg_conn_param_t conn_param = {
    .min_conn_interval = 16,
    .max_conn_interval = 60,
    .slave_latency = 0,
    .conn_sup_timeout = 400,
};
static const sid_ble_cfg_gatt_profile_t ble_profile[] = {
    {
        .service = ble_service,
        .char_count = countof(ble_characteristics),
        .characteristic = ble_characteristics,
        .desc_count = countof(ble_desc),
        .desc = ble_desc,
    },
};

static const sid_ble_config_t ble_cfg = {
    .name = "telink-dk",
    .mtu = 247,
    .is_adv_available = true,
    .mac_addr_type = SID_BLE_CFG_MAC_ADDRESS_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE,
    .adv_param = adv_param,
    .is_conn_available = true,
    .conn_param = conn_param,
    .num_profile = countof(ble_profile),
    .profile = ble_profile,
    .max_tx_power_in_dbm = 0,
    .enable_link_metrics = true,
    .metrics_msg_retries = 3,
};

static const sid_ble_link_config_t ble_config = {
    .create_ble_adapter = sid_pal_ble_adapter_create,
    .config = &ble_cfg,
};

const sid_ble_link_config_t* app_get_ble_config(void)
{
    return &ble_config;
}
