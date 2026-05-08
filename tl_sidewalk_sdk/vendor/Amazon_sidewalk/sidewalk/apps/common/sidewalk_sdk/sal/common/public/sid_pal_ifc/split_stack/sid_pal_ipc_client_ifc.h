/*
 * Copyright 2020-2023 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_IPC_CLIENT_IFC_H
#define SID_PAL_IPC_CLIENT_IFC_H

/** @file
 *
 * @defgroup sid_pal_ipc_ifc SID spit stack ipc interface
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details Provides ipc interface to be implemented by platform
 */

#include <stdint.h>
#include <sid_error.h>

#if SID_SDK_CONFIG_ENABLE_MAC_SPLIT_STACK

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*vendor_ipc_client_event_cb_t)(uint8_t ep, uint8_t event, void *ctx);
/**
 * initialize ipc context
 * @param ipc_ctx Pointer to ipc context
 * @return sid error
 */
void sid_pal_ipc_ctx_init(sid_mac_ipc_ctx_t* ipc_ctx);

/**
 * deinitialize ipc context
 * @param ipc_ctx Pointer to ipc context
 * @return sid error
 */
void sid_pal_ipc_ctx_deinit(sid_mac_ipc_ctx_t* ipc_ctx);

/**
 * register the callback function for events from server
 * @param callback calback for the events from server
 * @return sid error
 */
void sid_pal_ipc_mac_event_cb_subscribe(vendor_ipc_client_event_cb_t callback);

/**
 * send cmd to client
 * @param cmd_type command type
 * @param rsp_type response type
 * @param cmd_data pointer to command data
 * @param cmd_data_len command length
 * @return sid error
 */
sid_error_t sid_pal_ipc_send_cmd(uint8_t cmd_type, uint8_t rsp_type, uint8_t* cmd_data, uint16_t cmd_data_len)

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* SID_PAL_IPC_CLIENT_IFC_H */

#endif
