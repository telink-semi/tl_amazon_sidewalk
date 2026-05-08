/*
 * Copyright 2021-2024 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef PRINT_CMDS_H
#define PRINT_CMDS_H

#ifdef __cplusplus
extern "C" {
#endif

ace_status_t RnetFwVersionHandler(int32_t argc, const char **argv);
ace_status_t RnetAppVersionHandler(int32_t argc, const char **argv);
ace_status_t RnetCliPrintMfgHandler(int32_t argc, const char **argv);
ace_status_t RnetPrintNodeDB(int32_t argc, const char **argv);
ace_status_t RnetPrintMetrics(int32_t argc, const char **argv);
#if defined(GW_SUPPORT) && GW_SUPPORT
ace_status_t RnetPrintGatewayMetrics(int32_t argc, const char **argv);
#endif
ace_status_t RnetClearMetrics(int32_t argc, const char **argv);
#ifdef HALO_MEMINFO
ace_status_t RnetCliPrintMeminfo(int32_t argc, const char **argv);
#endif
ace_status_t halo_flash_prot_info_handler(int32_t argc, const char **argv);
#if !defined(GW_SUPPORT) || !GW_SUPPORT
ace_status_t sid_print_tx_id_handler(int32_t argc, const char **argv);
#endif

#ifdef __cplusplus
}
#endif

#endif
