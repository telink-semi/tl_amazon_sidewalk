/*
 * Copyright 2023 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.  This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef TIME_CMDS_V2_H
#define TIME_CMDS_V2_H

#ifdef __cplusplus
extern "C" {
#endif

ace_status_t sid_cli_time_now_handler(int32_t argc, const char **argv);
ace_status_t sid_cli_time_get_handler(int32_t argc, const char **argv);
ace_status_t sid_cli_time_set_handler(int32_t argc, const char **argv);
ace_status_t sid_cli_time_gcs_params(int32_t argc, const char **argv);
ace_status_t sid_cli_time_offset_handler(int32_t argc, const char **argv);

#ifdef __cplusplus
}
#endif

#endif
