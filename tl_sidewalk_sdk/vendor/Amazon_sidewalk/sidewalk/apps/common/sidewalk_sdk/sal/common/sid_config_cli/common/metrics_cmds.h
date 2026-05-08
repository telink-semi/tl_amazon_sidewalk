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

#ifndef METRICS_CMDS_H
#define METRICS_CMDS_H

#ifdef __cplusplus
extern "C" {
#endif

ace_status_t sid_cli_set_metrics_handler(int32_t argc, const char **argv);
ace_status_t sid_cli_send_metrics_handler(int32_t argc, const char **argv);

/**
 * @brief set fwk_v2 metrics values
 *          format:  <category_id> <metric_id> <value>
 *
 * @param argc
 * @param argv  inputs, like category individual metrics ID and correspond value
 * @return ace_status_t
 */
ace_status_t sid_cli_set_metrics_fwk_v2_handler(int32_t argc, const char **argv);

/**
 * @brief   set config via CLI (should be same functionality as it done via radio communication)
 *          format: set_config_fwk_v2 <action> <category_id> <metric_id>
 * @param argc
 * @param argv  inputs, like category individual metrics ID and correspond value
 * @return ace_status_t
 */
ace_status_t sid_cli_set_config_metrics_fwk_v2_handler(int32_t argc, const char **argv);
/**
 * @brief   triggers the reporting mechanism for periodic type reporting metrics.
 *          Does not clear metrics values.
 * @param argc
 * @param argv
 * @return ace_status_t
 */
ace_status_t sid_cli_trigger_metrics_fwk_v2_handler(int32_t argc, const char **argv);

#ifdef __cplusplus
}
#endif

#endif   // METRICS_CMDS_H
