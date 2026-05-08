/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <sid_pal_log_ifc.h>

#include <sx130x_halo.h>
#include <sx130x_hal.h>
#include <sx130x_radio.h>
#include <loragw_reg.h>
#include <loragw_hal.h>
#include <loragw_aux.h>
#include <loragw_com.h>
#include <loragw_sx1250.h>
#include <loragw_sx1302.h>
#include <loragw_sx1302_timestamp.h>

#include <stdlib.h>     /* qsort_r */
#include <stdint.h>     /* C99 types */
#include <stdbool.h>    /* bool type */
#include <stdio.h>      /* printf fprintf */
#include <string.h>     /* memcpy */
#include <inttypes.h>

#define CONTEXT_COM_TYPE  LGW_COM_SPI
#define CONTEXT_COM_PATH  ""

void lgw_sx1250_test(void)
{
    uint8_t test_buff[16];
    uint8_t read_buff[16];
    uint32_t test_val, read_val;
    int cycle_number = 0;
    int i, x;

    SID_HAL_LOG_INFO("Start of test for loragw_spi_sx1250.c\n");

    x = lgw_connect(CONTEXT_COM_TYPE, CONTEXT_COM_PATH);
    if (x != LGW_REG_SUCCESS) {
        SID_HAL_LOG_ERROR("ERROR: Failed to connect to the concentrator using COM %s\n", CONTEXT_COM_PATH);
        goto ret;
    }

    /* Reset radios */
    for (i = 0; i < LGW_RF_CHAIN_NB; i++) {
        sx1302_radio_reset(i, LGW_RADIO_TYPE_SX1250);
        sx1302_radio_set_mode(i, LGW_RADIO_TYPE_SX1250);
    }

    /* Select the radio which provides the clock to the sx1302 */
    sx1302_radio_clock_select(0);

    /* Ensure we can control the radio */
    lgw_reg_w(SX1302_REG_COMMON_CTRL0_HOST_RADIO_CTRL, 0x01);

    /* Ensure PA/LNA are disabled */
    lgw_reg_w(SX1302_REG_AGC_MCU_CTRL_FORCE_HOST_FE_CTRL, 1);
    lgw_reg_w(SX1302_REG_AGC_MCU_RF_EN_A_PA_EN, 0);
    lgw_reg_w(SX1302_REG_AGC_MCU_RF_EN_A_LNA_EN, 0);

    /* Set Radio in Standby mode */
    test_buff[0] = (uint8_t)STDBY_XOSC;
    x = sx1250_reg_w(SET_STANDBY, test_buff, 1, 0);
    if (x != LGW_REG_SUCCESS) {
        SID_HAL_LOG_ERROR("ERROR(%d): Failed to configure sx1250_0\n", __LINE__);
        goto ret;
    }
    x = sx1250_reg_w(SET_STANDBY, test_buff, 1, 1);
    if (x != LGW_REG_SUCCESS) {
        SID_HAL_LOG_ERROR("ERROR(%d): Failed to configure sx1250_1\n", __LINE__);
       goto ret;
    }
    wait_ms(10);

    test_buff[0] = 0x00;
    x = sx1250_reg_r(GET_STATUS, test_buff, 1, 0);
    if (x != LGW_REG_SUCCESS) {
        SID_HAL_LOG_ERROR("ERROR(%d): Failed to get sx1250_0 status\n", __LINE__);
        goto ret;
    }
    SID_HAL_LOG_INFO("Radio0: get_status: 0x%02X\n", test_buff[0]);
    x = sx1250_reg_r(GET_STATUS, test_buff, 1, 1);
    if (x != LGW_REG_SUCCESS) {
        SID_HAL_LOG_ERROR("ERROR(%d): Failed to get sx1250_1 status\n", __LINE__);
        goto ret;
    }
    SID_HAL_LOG_INFO("Radio1: get_status: 0x%02X\n", test_buff[0]);

    /* databuffer R/W stress test */
    while (cycle_number <= 10) {
        test_buff[0] = rand() & 0x7F;
        test_buff[1] = rand() & 0xFF;
        test_buff[2] = rand() & 0xFF;
        test_buff[3] = rand() & 0xFF;
        test_val = (test_buff[0] << 24) | (test_buff[1] << 16) | (test_buff[2] << 8) | (test_buff[3] << 0);
        sx1250_reg_w(SET_RF_FREQUENCY, test_buff, 4, 0);

        read_buff[0] = 0x08;
        read_buff[1] = 0x8B;
        read_buff[2] = 0x00;
        read_buff[3] = 0x00;
        read_buff[4] = 0x00;
        read_buff[5] = 0x00;
        read_buff[6] = 0x00;
        sx1250_reg_r(READ_REGISTER, read_buff, 7, 0);
        read_val = (read_buff[3] << 24) | (read_buff[4] << 16) | (read_buff[5] << 8) | (read_buff[6] << 0);
        if (read_val != test_val) {
            SID_HAL_LOG_ERROR("Cycle %d > \n", cycle_number);
            SID_HAL_LOG_ERROR("error during the buffer comparison\n");
            SID_HAL_LOG_ERROR("Written value: %08lX\n", test_val);
            SID_HAL_LOG_ERROR("Read value:    %08lX\n", read_val);
            goto ret;
        } else {
            ++cycle_number;
        }
    }
    SID_HAL_LOG_INFO("End of test for loragw_spi_sx1250.c\n");

ret:

    lgw_disconnect();
    return;
}

#include <time.h>

#define BUFF_SIZE_SPI       1024

#define SX1302_AGC_MCU_MEM  0x0000
#define SX1302_REG_COMMON   0x5600
#define SX1302_REG_AGC_MCU  0x5780

#define RAND_RANGE(min, max) (rand() % (max + 1 - min) + min)

void lgw_com_test(void)
{
    uint16_t max_buff_size = BUFF_SIZE_SPI;
    uint8_t *test_buff = NULL;
    uint8_t *read_buff = NULL;
    uint8_t data = 0;
    int cycle_number = 0;
    int i, x;
    uint16_t size;

    SID_HAL_LOG_INFO("Beginning of test for loragw_com.c \n");
    if ((test_buff = (uint8_t*)malloc(BUFF_SIZE_SPI * sizeof(uint8_t) +1)) == NULL) {
        SID_HAL_LOG_ERROR("ERROR: test_buff failed to malloc\n");
        goto ret;
    }
    if ((read_buff = (uint8_t*)malloc(BUFF_SIZE_SPI * sizeof(uint8_t) +1)) == NULL) {
        SID_HAL_LOG_ERROR("ERROR: read_buff failed to malloc\n");
        goto ret;
    }

    x = lgw_com_open(CONTEXT_COM_TYPE, CONTEXT_COM_PATH);
    if (x != 0) {
        SID_HAL_LOG_ERROR("ERROR: failed to open COM device %s\n", CONTEXT_COM_PATH);
        goto ret;
    }

    SID_HAL_LOG_INFO("max_buff_size %d \n", max_buff_size);

    /* normal R/W test */
    /* TODO */

    /* burst R/W test, small bursts << LGW_BURST_CHUNK */
    /* TODO */

    /* burst R/W test, large bursts >> LGW_BURST_CHUNK */
    /* TODO */

    x = lgw_com_r(LGW_SPI_MUX_TARGET_SX1302, SX1302_REG_COMMON + 6, &data);
    if (x != 0) {
        SID_HAL_LOG_ERROR("ERROR (%d): failed to read register\n", __LINE__);
        goto ret;
    }
    SID_HAL_LOG_INFO("SX1302 version: 0x%02X\n", data);

    x = lgw_com_r(LGW_SPI_MUX_TARGET_SX1302, SX1302_REG_AGC_MCU + 0, &data);
    if (x != 0) {
        SID_HAL_LOG_ERROR("ERROR (%d): failed to read register\n", __LINE__);
        goto ret;
    }
    x = lgw_com_w(LGW_SPI_MUX_TARGET_SX1302, SX1302_REG_AGC_MCU + 0, 0x06); /* mcu_clear, host_prog */
    if (x != 0) {
        SID_HAL_LOG_ERROR("ERROR (%d): failed to write register\n", __LINE__);
        goto ret;
    }

    srand(time(NULL));

    /* databuffer R/W stress test */
    while (cycle_number <= 8196) {
        /*************************************************
         *
         *      WRITE BURST TEST
         *
         * ***********************************************/

        size = 1 + rand() % max_buff_size;
        for (i = 0; i < size; ++i) {
            test_buff[i] = rand() & 0xFF;
        }
        /* Write burst with random data */
        x = lgw_com_wb(LGW_SPI_MUX_TARGET_SX1302, SX1302_AGC_MCU_MEM, test_buff, size);
        if (x != 0) {
            SID_HAL_LOG_ERROR("ERROR (%d): failed to write burst\n", __LINE__);
            goto ret;
        }
        /* Read back */
        x = lgw_com_rb(LGW_SPI_MUX_TARGET_SX1302, SX1302_AGC_MCU_MEM, read_buff, size);
        if (x != 0) {
            SID_HAL_LOG_ERROR("ERROR (%d): failed to read burst\n", __LINE__);
            goto ret;
        }
        /* Compare read / write buffers */
        for (i=0; ((i<size) && (test_buff[i] == read_buff[i])); ++i);
        if (i != size) {
            SID_HAL_LOG_ERROR("Cycle %d> size %d", cycle_number, size);
            SID_HAL_LOG_ERROR("error during the buffer comparison idx %d\n", i);
            /* Print what has been written */
            SID_HAL_LOG_ERROR("Written values: size %d\n", size);
            SID_HAL_LOG_HEXDUMP_INFO(test_buff, 16);
            /* Print what has been read back */
            SID_HAL_LOG_ERROR("Read values: size %d\n", size);
            SID_HAL_LOG_HEXDUMP_INFO(read_buff, 16);

            /* exit */
            goto ret;
        } else {
            //SID_HAL_LOG_INFO("did a %d-byte R/W on a data buffer with no error\n", size);
            ++cycle_number;
        }
        /*************************************************
         *
         *      WRITE SINGLE BYTE TEST
         *
         * ***********************************************/
        /* Single byte r/w test */

        test_buff[0] = rand() & 0xFF;
        /* Write single byte */
        x = lgw_com_w(LGW_SPI_MUX_TARGET_SX1302, SX1302_AGC_MCU_MEM, test_buff[0]);
        if (x != 0) {
            SID_HAL_LOG_ERROR("ERROR (%d): failed to write burst\n", __LINE__);
            goto ret;
        }
        /* Read back */
        x = lgw_com_r(LGW_SPI_MUX_TARGET_SX1302, SX1302_AGC_MCU_MEM, &read_buff[0]);
        if (x != 0) {
            SID_HAL_LOG_ERROR("ERROR (%d): failed to read burst\n", __LINE__);
            goto ret;
        }
        /* Compare read / write bytes */
        if (test_buff[0] != read_buff[0]) {
            SID_HAL_LOG_ERROR("Cycle %d> \n", cycle_number);
            SID_HAL_LOG_ERROR("error during the byte comparison\n");
            /* Print what has been written */
            SID_HAL_LOG_ERROR("Written value: %02X\n", test_buff[0]);
            /* Print what has been read back */
            SID_HAL_LOG_ERROR("Read values: %02X\n", read_buff[0]);
            /* exit */
            goto ret;
        } else {
            //("did a 1-byte R/W on a data buffer with no error\n");
            ++cycle_number;
        }

        /*************************************************
         *
         *      WRITE WITH BULK (USB only mode)
         *
         * ***********************************************/
        x = lgw_com_set_write_mode(LGW_COM_WRITE_MODE_BULK);
        if (x != 0) {
            SID_HAL_LOG_ERROR("ERROR (%d): failed to set bulk write mode\n", __LINE__);
            goto ret;
        }
        uint16_t num_req = RAND_RANGE(1, 254); /* keep one req for remaining bytes */
        size = RAND_RANGE(num_req, max_buff_size / 2); /* TODO: test proper limit */
        for (i = 0; i < size; i++) {
            test_buff[i] = rand() & 0xFF;
        }
        uint16_t size_per_req = size / num_req;
        uint16_t size_remaining = size - (num_req * size_per_req);
        uint16_t size_written = 0;
        for (i = 0; i < num_req; i++) {
            x = lgw_com_wb(LGW_SPI_MUX_TARGET_SX1302, SX1302_AGC_MCU_MEM + size_written, test_buff + size_written, size_per_req);
            if (x != 0) {
                SID_HAL_LOG_ERROR("ERROR (%d): failed to write burst\n", __LINE__);
                goto ret;
            }
            size_written += (size_per_req);
        }
        if (size_remaining > 0) {
            x = lgw_com_wb(LGW_SPI_MUX_TARGET_SX1302, SX1302_AGC_MCU_MEM + size_written, test_buff + size_written, size_remaining);
            if (x != 0) {
                SID_HAL_LOG_ERROR("ERROR (%d): failed to write burst\n", __LINE__);
                goto ret;
            }
        }
        /* Send data to MCU (UBS mode only) */
        x = lgw_com_flush();
        if (x != 0) {
            SID_HAL_LOG_ERROR("ERROR (%d): failed to flush write\n", __LINE__);
            goto ret;
        }
        /* Read back */
        x = lgw_com_rb(LGW_SPI_MUX_TARGET_SX1302, SX1302_AGC_MCU_MEM, read_buff, size);
        if (x != 0) {
            SID_HAL_LOG_ERROR("ERROR (%d): failed to read burst\n", __LINE__);
            goto ret;
        }
        /* Compare read / write buffers */
        for (i=0; ((i<size) && (test_buff[i] == read_buff[i])); ++i);
        if (i != size) {
            SID_HAL_LOG_ERROR("Cycle %d> size %d", cycle_number, size);
            SID_HAL_LOG_ERROR("error during the buffer comparison\n");
            /* Print what has been written */
            SID_HAL_LOG_ERROR("Written values: size %d\n",size);
            SID_HAL_LOG_HEXDUMP_INFO(test_buff, 16);
            /* Print what has been read back */
            SID_HAL_LOG_ERROR("Read values: size %d\n", size);
            SID_HAL_LOG_HEXDUMP_INFO(read_buff, 16);
            /* exit */
            goto ret;
        } else {
            //SID_HAL_LOG_INFO("did a %d-byte bulk R/W on a data buffer with no error\n", size);
            ++cycle_number;
        }
    }
    SID_HAL_LOG_INFO("End of test for loragw_com.c\n");

ret :
    if (test_buff) {
        free(test_buff);
    }
    if (read_buff) {
        free(read_buff);
    }
    lgw_com_close();
    return;
}

#include <math.h>

extern const struct lgw_reg_s loregs[LGW_TOTALREGS+1];
void lgw_reg_test(void)
{
    int x, i;
    int32_t val;
    bool error_found = false;
    uint8_t *rand_values = NULL;
    bool *reg_ignored = NULL; /* store register to be ignored */
    uint8_t reg_val;
    uint8_t reg_max;

    (void)error_found;

    SID_HAL_LOG_INFO("Start of test for loragw_reg.c LGW_TOTALREGS %d\n", LGW_TOTALREGS);

    if ((rand_values = (uint8_t*)malloc(LGW_TOTALREGS * sizeof(uint8_t) +1)) == NULL) {
        SID_HAL_LOG_ERROR("ERROR: rand_values failed to malloc\n");
        goto ret;
    }
    if ((reg_ignored = (bool*)malloc(LGW_TOTALREGS * sizeof(bool) +1)) == NULL) {
        SID_HAL_LOG_ERROR("ERROR: reg_ignored failed to malloc\n");
        goto ret;
    }
    x = lgw_connect(CONTEXT_COM_TYPE, CONTEXT_COM_PATH);
    if (x != LGW_REG_SUCCESS) {
        SID_HAL_LOG_ERROR("ERROR: failed to connect\n");
        goto ret;
    }
    /* The following registers cannot be tested this way */
    memset(reg_ignored, 0, sizeof reg_ignored);
    reg_ignored[SX1302_REG_COMMON_CTRL0_CLK32_RIF_CTRL] = true; /* all test fails if we set this one to 1 */
    /* Test 1: read all registers and check default value for non-read-only registers */
    SID_HAL_LOG_INFO("## TEST#1: read all registers and check default value for non-read-only registers\n");
    error_found = false;
    for (i = 0; i < LGW_TOTALREGS; i++) {
        if (loregs[i].rdon == 0) {
            x = lgw_reg_r(i, &val);
            if (x != LGW_REG_SUCCESS) {
                SID_HAL_LOG_ERROR("ERROR: failed to read register at index %d\n", i);
                goto ret;
            }
            if (val != loregs[i].dflt) {
                SID_HAL_LOG_ERROR("ERROR: default value for register at index %d is %d, should be %d\n", i, val, loregs[i].dflt);
                error_found = true;
            }
        }
    }
    SID_HAL_LOG_INFO("------------------\n");
    SID_HAL_LOG_INFO(" TEST#1 %s\n", (error_found == false) ? "PASSED" : "FAILED");
    SID_HAL_LOG_INFO("------------------\n\n");
    /* Test 2: read/write test on all non-read-only, non-pulse, non-w0clr, non-w1clr registers */
    SID_HAL_LOG_INFO("## TEST#2: read/write test on all non-read-only, non-pulse, non-w0clr, non-w1clr registers\n");
    /* Write all registers with a random value */
    error_found = false;
    for (i = 0; i < LGW_TOTALREGS; i++) {
        if ((loregs[i].rdon == 0) && (reg_ignored[i] == false)) {
            /* Peek a random value different form the default reg value */
            reg_max = pow(2, loregs[i].leng) - 1;
            if (loregs[i].leng == 1) {
                reg_val = !loregs[i].dflt;
            } else {
                /* ensure random value is not the default one */
                do {
                    if (loregs[i].sign == 1) {
                        reg_val = rand() % (reg_max / 2);
                    } else {
                        reg_val = rand() % reg_max;
                    }
                } while (reg_val == loregs[i].dflt);
            }
            /* Write selected value */
            x = lgw_reg_w(i, reg_val);
            if (x != LGW_REG_SUCCESS) {
                SID_HAL_LOG_ERROR("ERROR: failed to read register at index %d\n", i);
                goto ret;
            }
            /* store value for later check */
            rand_values[i] = reg_val;
        }
    }
    /* Read all registers and check if we got proper random value back */
    for (i = 0; i < LGW_TOTALREGS; i++) {
        if ((loregs[i].rdon == 0) && (loregs[i].chck == 1) && (reg_ignored[i] == false)) {
            x = lgw_reg_r(i, &val);
            if (x != LGW_REG_SUCCESS) {
                SID_HAL_LOG_ERROR("ERROR: failed to read register at index %d\n", i);
                goto ret;
            }
            /* check value */
            if (val != rand_values[i]) {
                SID_HAL_LOG_ERROR("ERROR: value read from register at index %d differs from the written value (w:%u r:%d)\n", i, rand_values[i], val);
                error_found = true;
            } else {
                //SID_HAL_LOG_INFO("INFO: MATCH reg %d (%u, %u)\n", i, rand_values[i], (uint8_t)val);
            }
        }
    }
    SID_HAL_LOG_INFO("------------------\n");
    SID_HAL_LOG_INFO(" TEST#2 %s\n", (error_found == false) ? "PASSED" : "FAILED");
    SID_HAL_LOG_INFO("------------------\n\n");
    x = lgw_disconnect();
    if (x != LGW_REG_SUCCESS) {
        SID_HAL_LOG_ERROR("ERROR: failed to disconnect\n");
        goto ret;
    }

ret:
    if (rand_values) {
        free(rand_values);
    }
    if (reg_ignored) {
        free(reg_ignored);
    }
    return;
}

void lgw_test(void)
{
    if (sx130x_reset(sx130x_get_drv_ctx()) != SX130X_STATUS_OK) {
        goto ret;
    }
    wait_ms(10);
    lgw_com_test();

    if (sx130x_reset(sx130x_get_drv_ctx()) != SX130X_STATUS_OK) {
        goto ret;
    }
    wait_ms(10);
    lgw_sx1250_test();

    if (sx130x_reset(sx130x_get_drv_ctx()) != SX130X_STATUS_OK) {
        goto ret;
    }
    wait_ms(10);
    lgw_reg_test();

    if (sx130x_reset(sx130x_get_drv_ctx()) != SX130X_STATUS_OK) {
        goto ret;
    }
    wait_ms(10);
ret:
    return;
}
