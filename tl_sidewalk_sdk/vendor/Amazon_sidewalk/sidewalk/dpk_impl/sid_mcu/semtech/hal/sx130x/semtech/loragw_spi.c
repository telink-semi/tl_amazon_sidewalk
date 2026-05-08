/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Host specific functions to address the LoRa concentrator registers through
    a SPI interface.
    Single-byte read/write and burst read/write.
    Could be used with multiple SPI ports in parallel (explicit file descriptor)

License: Revised BSD License, see LICENSE.TXT file include in the project
*/


/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

#include <sid_pal_log_ifc.h>

#include <sx130x_hal.h>
#include <sx130x_radio.h>
#include <loragw_spi.h>
#include <loragw_aux.h>

#include <stdint.h>     /* C99 types */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

#if DEBUG_COM == 1
    #define DEBUG_MSG(str)                SID_HAL_LOG_INFO(str)
    #define DEBUG_PRINTF(fmt, args...)    SID_HAL_LOG_INFO("%s:%d: "fmt, __FUNCTION__, __LINE__, args)
    #define CHECK_NULL(a)                 if(a==NULL) {SID_HAL_LOG_ERROR("%s:%d: ERROR: NULL POINTER AS ARGUMENT\n", __FUNCTION__, __LINE__);return LGW_SPI_ERROR;}
#else
    #define DEBUG_MSG(str)
    #define DEBUG_PRINTF(fmt, args...)
    #define CHECK_NULL(a)                 if(a==NULL) {return LGW_SPI_ERROR;}
#endif

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

#define READ_ACCESS      0x00
#define WRITE_ACCESS     0x80
#define LGW_BURST_CHUNK  1024 // Note: use radio device internal buffer.

#define LGW_TX_CMD_SIZE  3
#define LGW_RX_CMD_SIZE  4

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

/* SPI initialization and configuration */
int lgw_spi_open(const char * com_path, void **com_target_ptr)
{
    halo_drv_semtech_ctx_t *ctx = (halo_drv_semtech_ctx_t *)sx130x_get_drv_ctx();
    if (ctx->sx130x_cfg->bus_factory->create(&ctx->sx130x_bus_iface,
                ctx->sx130x_cfg->bus_factory->config) != SID_ERROR_NONE) {
        return LGW_SPI_ERROR;
    }
    *com_target_ptr = ctx;

    DEBUG_MSG("Note: SPI port opened and configured ok\n");

    return LGW_SPI_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* SPI release */
int lgw_spi_close(void *com_target)
{
    CHECK_NULL(com_target);
    //halo_drv_semtech_ctx_t *ctx = (halo_drv_semtech_ctx_t *)com_target;
    //ctx->sx130x_bus_iface->destroy(ctx->sx130x_bus_iface);
    return LGW_SPI_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Simple write */
int lgw_spi_w(void *com_target, uint8_t spi_mux_target, uint16_t address, uint8_t data)
{
    uint8_t cmd[LGW_TX_CMD_SIZE] = {0};
    uint8_t cmd_size;

    /* check input variables */
    CHECK_NULL(com_target);

    /* prepare frame to be sent */
    cmd[0] = spi_mux_target;
    cmd[1] = WRITE_ACCESS | ((address >> 8) & 0x7F);
    cmd[2] =                ((address >> 0) & 0xFF);
    cmd_size = LGW_TX_CMD_SIZE;

    if (sx130x_hal_write(com_target, cmd, cmd_size, &data,
                         sizeof(uint8_t)) != SX130X_STATUS_OK) {
        SID_HAL_LOG_ERROR("ERROR: SPI WRITE FAILURE\n");
        return LGW_SPI_ERROR;
    }

    return LGW_SPI_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Simple read */
int lgw_spi_r(void *com_target, uint8_t spi_mux_target, uint16_t address, uint8_t *data)
{
    uint8_t cmd[LGW_RX_CMD_SIZE];
    uint8_t cmd_size;

    /* check input variables */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    /* prepare frame to be sent */
    cmd[0] = spi_mux_target;
    cmd[1] = READ_ACCESS | ((address >> 8) & 0x7F);
    cmd[2] =               ((address >> 0) & 0xFF);
    cmd[3] = 0x00;
    cmd_size = LGW_RX_CMD_SIZE;

    if (sx130x_hal_read(com_target, cmd, cmd_size, data,
                        sizeof(uint8_t)) != SX130X_STATUS_OK) {
        SID_HAL_LOG_ERROR("ERROR: SPI READ FAILURE\n");
        return LGW_SPI_ERROR;
    }

    return LGW_SPI_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Single Byte Read-Modify-Write */
int lgw_spi_rmw(void *com_target, uint8_t spi_mux_target, uint16_t address,
                uint8_t offs, uint8_t leng, uint8_t data)
{
    int spi_stat = LGW_SPI_SUCCESS;
    uint8_t buf[4] = "\x00\x00\x00\x00";

    /* Read */
    spi_stat += lgw_spi_r(com_target, spi_mux_target, address, &buf[0]);

    /* Modify */
    buf[1] = ((1 << leng) - 1) << offs; /* bit mask */
    buf[2] = ((uint8_t)data) << offs; /* new data offsetted */
    buf[3] = (~buf[1] & buf[0]) | (buf[1] & buf[2]); /* mixing old & new data */

    /* Write */
    spi_stat += lgw_spi_w(com_target, spi_mux_target, address, buf[3]);

    return spi_stat;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Burst (multiple-byte) write */
int lgw_spi_wb(void *com_target, uint8_t spi_mux_target, uint16_t address,
               const uint8_t *data, uint16_t size)
{
    int32_t err = LGW_SPI_SUCCESS;
    uint8_t cmd[LGW_TX_CMD_SIZE] = {0};
    uint8_t cmd_size;

    /* check input parameters */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    if (size == 0) {
        SID_HAL_LOG_ERROR("ERROR: BURST OF NULL LENGTH\n");
        err = LGW_SPI_ERROR;
        goto ret;
    }

    /* prepare command byte */
    cmd[0] = spi_mux_target;
    cmd[1] = WRITE_ACCESS | ((address >> 8) & 0x7F);
    cmd[2] =                ((address >> 0) & 0xFF);
    cmd_size = LGW_TX_CMD_SIZE;

    uint16_t size_to_do = size;
    uint16_t offset = 0;
    uint16_t w_addr = address;
    while (size_to_do > LGW_BURST_CHUNK) {
        if (sx130x_hal_write(com_target, cmd, cmd_size, &data[offset],
                             LGW_BURST_CHUNK) != SX130X_STATUS_OK) {
            SID_HAL_LOG_ERROR("ERROR: SPI BURST WRITE FAILURE\n");
            err = LGW_SPI_ERROR;
            goto ret;
        }
        w_addr += LGW_BURST_CHUNK;
        offset += LGW_BURST_CHUNK;
        size_to_do -= LGW_BURST_CHUNK;

        //update address
        cmd[1] = WRITE_ACCESS | ((w_addr >> 8) & 0x7F);
        cmd[2] =                ((w_addr >> 0) & 0xFF);
    }
    if (size_to_do > 0) {
        if (sx130x_hal_write(com_target, cmd, cmd_size, &data[offset],
                             size_to_do) != SX130X_STATUS_OK) {
            SID_HAL_LOG_ERROR("ERROR: SPI BURST WRITE FAILURE\n");
            err = LGW_SPI_ERROR;
            goto ret;
        }
    }

ret:
    return err;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Burst (multiple-byte) read */
int lgw_spi_rb(void *com_target, uint8_t spi_mux_target, uint16_t address,
               uint8_t *data, uint16_t size)
{
    int32_t err = LGW_SPI_SUCCESS;
    uint8_t cmd[LGW_RX_CMD_SIZE] = {0};
    uint8_t cmd_size;

    /* check input parameters */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    if (size == 0) {
        SID_HAL_LOG_ERROR("ERROR: BURST OF NULL LENGTH\n");
        err = LGW_SPI_ERROR;
        goto ret;
    }

    /* prepare command byte */
    cmd[0] = spi_mux_target;
    cmd[1] = READ_ACCESS | ((address >> 8) & 0x7F);
    cmd[2] =               ((address >> 0) & 0xFF);
    cmd[3] = 0x00;
    cmd_size = LGW_RX_CMD_SIZE;

    uint16_t size_to_do = size;
    uint16_t offset = 0;
    uint16_t r_addr = address;
    while (size_to_do > LGW_BURST_CHUNK) {
        if (sx130x_hal_read(com_target, cmd, cmd_size, &data[offset],
                            LGW_BURST_CHUNK) != SX130X_STATUS_OK) {
            SID_HAL_LOG_ERROR("ERROR: SPI BURST READ FAILURE\n");
            err = LGW_SPI_ERROR;
            goto ret;
        }
        r_addr += LGW_BURST_CHUNK;
        offset += LGW_BURST_CHUNK;
        size_to_do -= LGW_BURST_CHUNK;

        //update address
        cmd[1] = READ_ACCESS | ((r_addr >> 8) & 0x7F);
        cmd[2] =               ((r_addr >> 0) & 0xFF);
    }
    if (size_to_do > 0) {
        if (sx130x_hal_read(com_target, cmd, cmd_size, &data[offset],
                            size_to_do) != SX130X_STATUS_OK) {
            SID_HAL_LOG_ERROR("ERROR: SPI BURST READ FAILURE\n");
            err = LGW_SPI_ERROR;
            goto ret;
        }
    }

ret:
    return err;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

uint16_t lgw_spi_chunk_size(void)
{
    return (uint16_t)LGW_BURST_CHUNK;
}

/* --- EOF ------------------------------------------------------------------ */
