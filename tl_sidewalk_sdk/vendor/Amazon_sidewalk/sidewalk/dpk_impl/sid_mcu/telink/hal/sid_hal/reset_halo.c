/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#include <sid_hal_reset_ifc.h>


static void reset(halo_system_reset_type_t type)
{
    void *pc = halo_get_pc_address();
    const halo_system_reset_meta_info_t info = {
       .file_name     =  "sid_hal_reset",
       .line_number   =  __LINE__,
       .pc = pc
    };

    halo_system_reset(&info, type);
}

sid_error_t sid_hal_reset(sid_hal_reset_type_t type)
{
    switch(type) {
        case SID_HAL_RESET_NORMAL:
            reset(HALO_SYSTEM_RESET_TYPE_NORMAL);
            break;
        case SID_HAL_RESET_DFU:
            halo_platform_enable_dfu_mode();
            break;
        default:
            return SID_ERROR_NOSUPPORT;
    }

    return SID_ERROR_NONE;
}
