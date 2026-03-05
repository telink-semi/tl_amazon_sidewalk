/*
 * Copyright 2023-2024 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef PRINT_V2_CMDS_H
#define PRINT_V2_CMDS_H

#include <sid_utils.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Function to print manufacturing values
 *
 * @param[in] argc arguments count
 * @param[in] argv array of arguments.
 * @retval ACE_STATUS_OK when operation completed successfully.
 */
ace_status_t sid_cli_print_mfg_handler(int32_t argc, const char **argv);

/**
 * @brief Function to print firmware version
 *
 * @param[in] argc arguments count
 * @param[in] argv array of arguments.
 * @retval ACE_STATUS_OK when operation completed successfully.
 */
ace_status_t sid_cli_fw_version_handler(int32_t argc, const char **argv);

/**
 * @brief Function to print application version
 *
 * @param[in] argc arguments count
 * @param[in] argv array of arguments.
 * @retval ACE_STATUS_OK when operation completed successfully.
 */
ace_status_t sid_cli_app_version_handler(int32_t argc, const char **argv);

/**
 * @brief Function to print tx id
 *
 * @param[in] argc arguments count
 * @param[in] argv array of arguments.
 * @retval ACE_STATUS_OK when operation completed successfully.
 */
ace_status_t sid_cli_print_tx_id_handler(int32_t argc, const char **argv);

/**
 * @brief   Function to print metrics_fwk_v2
 *          command should have format:
 *          print metrics_fwk_v2 <categoty_id>
 * @param argc  amount of input vars
 * @param argv  pointer to input vars
 * @return ace_status_t
 */
ace_status_t sid_cli_print_metrics_fwk_v2_handler(int32_t argc, const char **argv);

/**
 * @brief  function called from CLI to clear defined category metrics
 *          command should have format:
 *          print clear_metrics_fwk_v2 <category> <metrics_id>
 *          in case <metrics_id> is not set all the metrics for selected
 *          category will be cleared
 *
 * @param argc  amount of input vars
 * @param argv  pointer to input vars
 * @return ace_status_t
 */
ace_status_t sid_cli_clear_metrics_fwk_v2_handler(int32_t argc, const char **argv);

/**
 * @brief  function called from CLI to get capability and config
 *
 * @param argc  amount of input vars
 * @param argv  pointer to input vars
 * @return ace_status_t
 */
ace_status_t sid_cli_print_cap_cfg(int32_t argc, const char **argv);

/**
 * @brief  function called from CLI to clear capability and config from kv store
 *
 * @param argc  amount of input vars
 * @param argv  pointer to input vars
 * @return ace_status_t
 */
ace_status_t sid_cli_clear_cap_cfg(int32_t argc, const char **argv);


#ifdef __cplusplus
}
#endif

#endif
