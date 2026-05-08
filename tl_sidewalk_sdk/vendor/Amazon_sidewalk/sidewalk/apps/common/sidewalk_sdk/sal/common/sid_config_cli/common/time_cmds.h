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

#ifndef TIME_CMDS_H
#define TIME_CMDS_H

#ifdef __cplusplus
extern "C" {
#endif

ace_status_t RnetCliGCSParamHandler(int32_t argc, const char **argv);
ace_status_t RnetCliSendNotifyTimeHandler(int32_t argc, const char **argv);
ace_status_t RnetCliSendGetTimeHandler(int32_t argc, const char **argv);
ace_status_t RnetCliSetRtcPPMHandler(int32_t argc, const char **argv);
ace_status_t RnetCliSetTimeHandler(int32_t argc, const char **argv);
ace_status_t RnetCliTimeNowHandler(int32_t argc, const char **argv);
ace_status_t RnetCliSetTimeOffsetHandler(int32_t argc, const char **argv);

#ifdef __cplusplus
}
#endif

#endif
