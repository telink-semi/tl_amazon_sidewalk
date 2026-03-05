/*
 * Copyright 2021-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#include <sid_pal_delay_ifc.h>

#include <sx130x_halo.h>
#include <sx130x_radio.h>

static int32_t sx130x_hal_rdwr(const void *context,
                               const uint8_t *command, const uint16_t command_length,
                               uint8_t *data, const uint16_t data_length, bool read)
{
    int32_t err;

    do {
        if (context == NULL) {
            err = RADIO_ERROR_INVALID_PARAMS;
            break;
        }

        const halo_drv_semtech_ctx_t *ctx = (halo_drv_semtech_ctx_t *)context;
        if (ctx->sx130x_cfg->internal_buffer.p == NULL || command == NULL) {
            err = RADIO_ERROR_INVALID_PARAMS;
            break;
        }
        if (ctx->sx130x_cfg->internal_buffer.size < (data_length + command_length)) {
            err = RADIO_ERROR_INVALID_PARAMS;
            break;
        }

        memcpy(ctx->sx130x_cfg->internal_buffer.p, command, command_length);
        if (data != NULL) {
            memcpy(&ctx->sx130x_cfg->internal_buffer.p[command_length], data, data_length);
        }
        if (ctx->sx130x_bus_iface->xfer(ctx->sx130x_bus_iface, &ctx->sx130x_cfg->bus_selector,
                 ctx->sx130x_cfg->internal_buffer.p, ctx->sx130x_cfg->internal_buffer.p,
                 data_length + command_length) != SID_ERROR_NONE) {
            err = SX130X_STATUS_ERROR;
            break;
        }
        if (read) {
            memcpy(data ,&ctx->sx130x_cfg->internal_buffer.p[command_length], data_length);
        }
        err = SX130X_STATUS_OK;
    } while (0);

    return err;
}

sx130x_status_t sx130x_hal_reset(const void *ctx)
{
    const halo_drv_semtech_ctx_t *drv_ctx;
    int32_t err;

    do {
        if (NULL == ctx) {
            err = RADIO_ERROR_INVALID_PARAMS;
            break;
        }

        drv_ctx = (halo_drv_semtech_ctx_t *)ctx;
        err = RADIO_ERROR_HARDWARE_ERROR;
        if (drv_ctx->sx130x_cfg->gpios.power != HALO_GPIO_NOT_CONNECTED) {
            if (sid_pal_gpio_set_direction(drv_ctx->sx130x_cfg->gpios.power,
                SID_PAL_GPIO_DIRECTION_OUTPUT) != SID_ERROR_NONE) {
                break;
            }
            sid_pal_delay_us(100*1000);
            if (sid_pal_gpio_write(drv_ctx->sx130x_cfg->gpios.power, 1) != SID_ERROR_NONE) {
                break;
            }
            sid_pal_delay_us(100*1000);
            if (sid_pal_gpio_write(drv_ctx->sx130x_cfg->gpios.power, 0) != SID_ERROR_NONE) {
                break;
            }
            sid_pal_delay_us(100*1000);
            err = RADIO_ERROR_NONE;
        }
    } while (0);

    if (err != RADIO_ERROR_NONE) {
        return SX130X_STATUS_ERROR;
    }

    return SX130X_STATUS_OK;
}

sx130x_status_t sx130x_hal_read(const void *context,
                                const uint8_t *command, const uint16_t command_length,
                                uint8_t *data, const uint16_t data_length)
{
    sx130x_status_t status = SX130X_STATUS_ERROR;

    do {
        if (context == NULL || command == NULL || data == NULL ||
            command_length == 0 || data_length == 0) {
            break;
        }

        if (sx130x_hal_rdwr(context, command, command_length,
                            (uint8_t *)data, data_length, true) != RADIO_ERROR_NONE) {
            break;
        }
        status = SX130X_STATUS_OK;
    } while (0);

    return status;
}

sx130x_status_t sx130x_hal_write(const void *context,
                                 const uint8_t *command, const uint16_t command_length,
                                 const uint8_t *data, const uint16_t data_length)
{
    sx130x_status_t status = SX130X_STATUS_ERROR;

    do {
        //For write data can be null and data length 0
        if (context == NULL || command == NULL || command_length == 0) {
            break;
        }

        if (sx130x_hal_rdwr(context, command, command_length,
                            (uint8_t *)data, data_length, false) != RADIO_ERROR_NONE) {
            break;
        }
        status = SX130X_STATUS_OK;
    } while (0);

    return status;
}
