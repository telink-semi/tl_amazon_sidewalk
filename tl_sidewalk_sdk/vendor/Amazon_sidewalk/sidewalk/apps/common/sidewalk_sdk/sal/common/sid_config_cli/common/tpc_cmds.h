/*
 * Copyright 2025 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef TPC_CMDS_H
#define TPC_CMDS_H

#ifdef __cplusplus
extern "C" {
#endif

ace_status_t sid_cli_lora_tpc_get_status(int32_t argc, const char **argv);
ace_status_t sid_cli_lora_tpc_get_config(int32_t argc, const char **argv);
ace_status_t sid_cli_lora_tpc_set_config(int32_t argc, const char **argv);

ace_status_t sid_cli_fsk_tpc_get_status(int32_t argc, const char **argv);
ace_status_t sid_cli_fsk_tpc_get_config(int32_t argc, const char **argv);
ace_status_t sid_cli_fsk_tpc_set_config(int32_t argc, const char **argv);

#ifdef __cplusplus
}
#endif

#endif /* TPC_CMDS_H */
