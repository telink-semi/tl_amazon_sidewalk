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
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include <sid_hal_reset_ifc.h>


sid_error_t sid_hal_reset(sid_hal_reset_type_t type)
{
    if (type == SID_HAL_RESET_NORMAL) {
          sys_reboot();
    } else {
        return SID_ERROR_NOSUPPORT;
    }

    return SID_ERROR_NONE;
}
