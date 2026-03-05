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

#ifndef MAC_CMDS_H
#define MAC_CMDS_H

#ifdef __cplusplus
extern "C" {
#endif

ace_status_t RnetCliMACGetCfgHandler(int32_t argc, const char **argv);
ace_status_t RnetCliMACSetCfgHandler(int32_t argc, const char **argv);
ace_status_t RnetCliMACSetKeyHandler(int32_t argc, const char **argv);
ace_status_t RnetCliMACResetCfgHandler(int32_t argc, const char **argv);
ace_status_t RnetCliMacLogHandler(int32_t argc, const char **argv);
ace_status_t RnetCliMACSetDevProfile(int32_t argc, const char **argv);
#if !HALO_WITH_MODULE_HALO_LIB_HALO_MANAGEMENT
ace_status_t RnetCliMACGetDevProfile(int32_t argc, const char **argv);
ace_status_t RnetCliLinkqHandler2(int32_t argc, const char **argv);
#endif
#if 0
ace_status_t RnetCliLinkqHandler(int32_t argc, const char **argv);
#endif

#ifdef __cplusplus
}
#endif

#endif
