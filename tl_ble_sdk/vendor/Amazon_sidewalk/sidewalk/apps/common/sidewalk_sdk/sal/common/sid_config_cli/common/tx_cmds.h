/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef TX_CMDS_H
#define TX_CMDS_H

#ifdef __cplusplus
extern "C" {
#endif

#if SID_MAC_V2
ace_status_t sid_cli_tx_rnet_handler(int32_t argc, const char **argv);
ace_status_t sid_cli_form_app_cmd_handler(int32_t argc, const char **argv);
ace_status_t sid_cli_set_get_rtc_cmd_handler(int32_t argc, const char **argv);
ace_status_t sid_cli_tx_dcr_cmd_handler(int32_t argc, const char **argv);
#else
ace_status_t RnetCliTxRnetHandler(int32_t argc, const char **argv);
ace_status_t RnetCliFormAppCmdHandler(int32_t argc, const char **argv);
ace_status_t RnetCliSetGetRtcCmdHandler(int32_t argc, const char **argv);
ace_status_t RnetCliSetDcrCmdHandler(int32_t argc, const char **argv);
#endif

#ifdef __cplusplus
}
#endif

#endif
