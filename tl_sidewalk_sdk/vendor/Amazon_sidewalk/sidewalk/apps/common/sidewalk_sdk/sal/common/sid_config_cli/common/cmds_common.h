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

#ifndef MAC_CMDS_COMMON_H
#define MAC_CMDS_CONNON_H

#include <sid_error.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CLI_HELP_STRING
#define CLI_HELP_STRING "Refer to \"Product CLI Mapping Functions\" wiki"
#endif

#ifdef SYNTAX_ERR
#undef SYNTAX_ERR
#endif
#define SYNTAX_ERR      "%s: Syntax err \r\n"

#define FLOAT_TO_LOGS(val) (uint32_t)(((val) < 0 && (val) > -1.0) ? "-" : ""),   \
                             (int32_t)(val),                                     \
                             (int32_t)((((val) > 0) ? (val) - (int32_t)(val)     \
                                                  : (int32_t)(val) - (val))*100)

bool cli_check_arguments(int32_t minimum_invalid_argc, int32_t argc, const char ** argv);

#ifdef __cplusplus
}
#endif

#endif
