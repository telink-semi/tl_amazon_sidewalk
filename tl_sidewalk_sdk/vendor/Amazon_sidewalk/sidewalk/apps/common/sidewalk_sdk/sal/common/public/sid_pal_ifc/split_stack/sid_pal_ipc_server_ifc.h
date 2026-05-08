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

#ifndef SID_PAL_IPC_SERVER_IFC_H
#define SID_PAL_IPC_SERVER_IFC_H

/** @file
 *
 * @defgroup sid_pal_ipc_ifc SID spit stack ipc interface
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details Provides ipc interface to be implemented by platform
 */

#include <stdint.h>

#if SID_SDK_CONFIG_ENABLE_MAC_SPLIT_STACK

#ifdef __cplusplus
extern "C" {
#endif

/**
 * send event from server to client
 * @param event event
 */
void sid_pal_ipc_send_event(uint8_t event);

/**
 * send response from server to client
 * @param data pointer to response data
 * @param size response size
 */
void sid_pal_ipc_send_response( const void* data, size_t size);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* SID_PAL_IPC_SERVER_IFC_H */

#endif
