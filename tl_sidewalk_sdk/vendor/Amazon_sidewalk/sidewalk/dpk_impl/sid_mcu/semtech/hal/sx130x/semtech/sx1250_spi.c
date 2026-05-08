/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Functions used to handle LoRa concentrator SX1250 radios.

License: Revised BSD License, see LICENSE.TXT file include in the project
*/


/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

#include <sid_pal_log_ifc.h>

#include <sx130x_hal.h>
#include <loragw_spi.h>
#include <loragw_aux.h>
#include <sx1250_spi.h>

#include <stdint.h>     /* C99 types */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

#if DEBUG_RAD == 1
    #define DEBUG_MSG(str)                SID_HAL_LOG_INFO(str)
    #define DEBUG_PRINTF(fmt, args...)    SID_HAL_LOG_INFO("%s:%d: "fmt, __FUNCTION__, __LINE__, args)
    #define CHECK_NULL(a)                 if(a==NULL){SID_HAL_LOG_ERROR("%s:%d: ERROR: NULL POINTER AS ARGUMENT\n", __FUNCTION__, __LINE__);return LGW_SPI_ERROR;}
#else
    #define DEBUG_MSG(str)
    #define DEBUG_PRINTF(fmt, args...)
    #define CHECK_NULL(a)                 if(a==NULL){return LGW_SPI_ERROR;}
#endif

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

#define WAIT_BUSY_SX1250_MS  1
#define SX1250_CMD_SIZE      2
/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

int sx1250_spi_w(void *com_target, uint8_t spi_mux_target, sx1250_op_code_t op_code,
                 uint8_t *data, uint16_t size)
{
    uint8_t cmd_size = SX1250_CMD_SIZE; /* header + op_code */
    uint8_t cmd[SX1250_CMD_SIZE] = {0};

    /* wait BUSY */
    wait_ms(WAIT_BUSY_SX1250_MS);

    /* check input variables */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    /* prepare frame to be sent */
    cmd[0] = spi_mux_target;
    cmd[1] = (uint8_t)op_code;

    if (sx130x_hal_write(com_target, cmd, cmd_size, data, size) != SX130X_STATUS_OK) {
        SID_HAL_LOG_ERROR("ERROR: SPI WRITE FAILURE\n");
        return LGW_SPI_ERROR;
    }

    return LGW_SPI_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int sx1250_spi_r(void *com_target, uint8_t spi_mux_target, sx1250_op_code_t op_code,
                 uint8_t *data, uint16_t size)
{
    uint8_t cmd_size = SX1250_CMD_SIZE; /* header + op_code + NOP */
    uint8_t cmd[SX1250_CMD_SIZE] = {0};

    /* wait BUSY */
    wait_ms(WAIT_BUSY_SX1250_MS);

    /* check input variables */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    /* prepare frame to be sent */
    cmd[0] = spi_mux_target;
    cmd[1] = (uint8_t)op_code;

    if (sx130x_hal_read(com_target, cmd, cmd_size, data, size) != SX130X_STATUS_OK) {
        SID_HAL_LOG_ERROR("ERROR: SPI READ FAILURE\n");
        return LGW_SPI_ERROR;
    }

    return LGW_SPI_SUCCESS;
}

/* --- EOF ------------------------------------------------------------------ */
