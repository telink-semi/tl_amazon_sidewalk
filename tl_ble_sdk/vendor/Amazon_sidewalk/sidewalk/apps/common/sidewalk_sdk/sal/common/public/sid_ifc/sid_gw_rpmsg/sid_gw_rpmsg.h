/*
 * Copyright 2023-2025 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_GW_RPMSG_H
#define SID_GW_RPMSG_H

/// @cond sid_ifc_gw_en

/** @file
 *
 * @defgroup Sidewalk_GW_API Sidewalk GW API
 * @brief API for communicating with the Sidewalk network
 * @{
 * @ingroup  SIDEWALK_GW_API
 */

#include <sid_error.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SID_GW_RPMSG_ADDR_ANY 0xffffffff

struct sid_gw_rpmsg_ept;

/**
 * @brief SID GW RPMsg callback handler type.
 * @param [in] ept pointer to SID GW RPMsg endpoint
 * @param [in] priv private pointer for callback
 * @param [in] src source address of received RPMsg
 * @param [in] buf pointer to raw data
 * @param [in] len size of raw data at @p buf
 * @return Upon successful completion, a value of 0 is returned.
 *         Otherwise, a value of -1 is returned.
 */
typedef int (*sid_gw_rpmsg_cb_t)(const struct sid_gw_rpmsg_ept *ept, void *priv,
                                 uint32_t src, const void *buf, size_t len);

/**
 * @brief Register SID GW RPMsg endpoint.
 * @param [in] name endpoint name
 * @param [in] dst destination address
 * @param [in] src source address
 * @param [in] cb callback handler
 * @param [in] priv private pointer for callback
 * @return Upon successful completion, a pointer to SID GW RPMsg endpoint.
 *         Otherwise, a value of NULL is returned.
 */
struct sid_gw_rpmsg_ept *sid_gw_rpmsg_register(const char *name,
                                               uint32_t dst, uint32_t src,
                                               sid_gw_rpmsg_cb_t cb,
                                               void *priv);

/**
 * @brief Unregister SID GW RPMsg endpoint.
 * @param [in] ept SID GW RPMsg endpoint
 */
void sid_gw_rpmsg_unregister(struct sid_gw_rpmsg_ept *ept);

/**
 * @brief Send RPMsg from specified source to specified destination.
 * @param [in] dst destination address
 * @param [in] src source address
 * @param [in] buf pointer to raw data
 * @param [in] len size of raw data at @p buf
 * @return Upon successful completion, a value of 0 is returned.
 *         Otherwise, a value of -1 is returned.
 */
int sid_gw_rpmsg_send_offchannel(uint32_t dst, uint32_t src,
                                 const void *buf, size_t len);

/**
 * @brief Send RPMsg to specified destination.
 * @param [in] ept SID GW RPMsg endpoint
 * @param [in] dst destination address
 * @param [in] buf pointer to raw data
 * @param [in] len size of raw data at @p buf
 * @return Upon successful completion, a value of 0 is returned.
 *         Otherwise, a value of -1 is returned.
 */
int sid_gw_rpmsg_sendto(const struct sid_gw_rpmsg_ept *ept, uint32_t dst,
                        const void *buf, size_t len);

/**
 * @brief Send RPMsg.
 * @param [in] ept SID GW RPMsg endpoint
 * @param [in] buf pointer to raw data
 * @param [in] len size of raw data at @buf
 * @return Upon successful completion, a value of 0 is returned.
 *         Otherwise, a value of -1 is returned.
 */
int sid_gw_rpmsg_send(const struct sid_gw_rpmsg_ept *ept,
                      const void *buf, size_t len);

/*
 * @brief Clear i/o buffers and reset connection.
 * @return Upon successful completion, a value of 0 is returned.
 *         Otherwise, a value of -1 is returned.
 */
int sid_gw_rpmsg_purge(void);

/*
 * @brief Suspend HTP
 * @return Upon successful completion, a value of 0 is returned.
 *         Otherwise, a value of -1 is returned.
 */
int sid_gw_rpmsg_suspend(void);

#ifdef __cplusplus
}
#endif

/** @} */

/// @endcond

#endif /*! SID_GW_RPMSG_H */
