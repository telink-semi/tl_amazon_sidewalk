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

#ifndef MAC_V2_CMDS_H
#define MAC_V2_CMDS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Handler for GetCfg command - queries all cfg parameters
 *
 * @param[in] argc arguments count
 * @param[in] argv array of arguments.
 * @retval ACE_STATUS_OK when operation completed successfully.
 */
ace_status_t sid_cli_mac_get_cfg_handler(int32_t argc, const char **argv);

/**
 * @brief Main function to set cfg parameters
 *
 * @param[in] argc arguments count
 * @param[in] argv array of arguments.
 * @retval ACE_STATUS_OK when operation completed successfully.
 */
ace_status_t sid_cli_mac_set_cfg_handler(int32_t argc, const char **argv);

/**
 * @brief Helper function for SetKeys for the user
 * and parse main command
 *
 * @param[in] argc arguments count
 * @param[in] argv array of arguments.
 * @retval ACE_STATUS_OK when operation completed successfully.
 */
ace_status_t sid_cli_mac_set_key_handler(int32_t argc, const char **argv);

/**
 * @brief Function to reset all config params
 *
 * @param[in] argc arguments count
 * @param[in] argv array of arguments.
 * @retval ACE_STATUS_OK when operation completed successfully.
 */
ace_status_t sid_cli_mac_reset_cfg_handler(int32_t argc, const char **argv);

/**
 * @brief Function to enable/disable log messages
 *
 * @param[in] argc arguments count
 * @param[in] argv array of arguments.
 * @retval ACE_STATUS_OK when operation completed successfully.
 */
ace_status_t sid_cli_mac_log_handler(int32_t argc, const char **argv);
#ifdef __cplusplus
}
#endif

#endif //MAC_V2_CMDS_H
