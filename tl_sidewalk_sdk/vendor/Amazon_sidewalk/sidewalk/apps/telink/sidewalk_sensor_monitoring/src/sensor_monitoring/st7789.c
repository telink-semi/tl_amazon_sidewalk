/********************************************************************************************************
 * @file    st7789.c
 *
 * @brief   This is the source file for Telink RISC-V MCU
 *
 * @author  Driver Group
 * @date    2024
 *
 * @par     Copyright (c) 2024, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/
#include "sensor_monitoring/st7789.h"
#include "sensor_monitoring/font.h"

#include "tl_common.h"

#if CONFIG_SIDEWALK_SUBGHZ_SUPPORT // Only one SPI, used to control sub-Giga radio

void LCD_Init(void) {
}


void LCD_Fill(uint16_t color) {

}

void LCD_2IN_DrawPaint(uint16_t x, uint16_t y, uint16_t Color) {

}

void LCD_ClearWindow(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, uint16_t color) {

}

void LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint8_t *p) {

}

void LCD_showText(char *text) {
}

#else



LCD_ST7789_ATTRIBUTES LCD_ST7789_ST;

uint16_t POINT_COLOR = GREEN;
uint16_t BACK_COLOR  = BLACK; //BG


#define SPI_MODULE GSPI_MODULE
gspi_pin_config_t gspi_pin_config = {
    .spi_csn_pin      = SPI_CSN_PIN,
    .spi_clk_pin      = SPI_CLK_PIN,
    .spi_mosi_io0_pin = SPI_MOSI_PIN,
    .spi_miso_io1_pin = (gpio_func_pin_e)GPIO_NONE_PIN, //3line mode is required, otherwise it is NONE_PIN.
    .spi_io2_pin      = (gpio_func_pin_e)GPIO_NONE_PIN, //quad  mode is required, otherwise it is NONE_PIN.
    .spi_io3_pin      = (gpio_func_pin_e)GPIO_NONE_PIN, //quad  mode is required, otherwise it is NONE_PIN.
};

spi_wr_rd_config_t spi_b91m_slave_protocol_config = {
    .spi_io_mode     = SPI_3_LINE_MODE, /*IO mode set to SPI_3_LINE_MODE when SPI_3LINE_SLAVE.*/
    .spi_dummy_cnt   = 0,               //B92 supports up to 32 clk cycle dummy, and TL751X,TL7518,TL721X,TL321X,TL322X supports up to 256 clk cycle dummy.
    .spi_cmd_en      = 0,
    .spi_addr_en     = 0,
    .spi_addr_len    = 0, // when spi_addr_en = 0,invalid set.
    .spi_cmd_fmt_en  = 0, // when spi_cmd_en  = 0,invalid set.
    .spi_addr_fmt_en = 0, // when spi_addr_en = 0,invalid set.
};

void LCD_ST7789_SendCommand(uint8_t cmd)
{
    LCD_ST7789_DC_0;
    spi_master_write_plus(SPI_MODULE, SPI_WRITE_DATA_SINGLE_CMD, (uint32_t)NULL, (uint8_t *)&cmd, 1, SPI_MODE_WR_WRITE_ONLY);
}

void LCD_ST7789_SendByte(uint8_t data)
{
    LCD_ST7789_DC_1;
    spi_master_write_plus(SPI_MODULE, SPI_WRITE_DATA_SINGLE_CMD, (uint32_t)NULL, (uint8_t *)&data, 1, SPI_MODE_WR_WRITE_ONLY);
}

void LCD_ST7789_SendBytes(uint8_t *data, uint32_t len)
{
    LCD_ST7789_DC_1;
    spi_master_write_plus(SPI_MODULE, SPI_WRITE_DATA_SINGLE_CMD, (uint32_t)NULL, data, len, SPI_MODE_WR_WRITE_ONLY);
}

static void LCD_ST7789_Reset(void)
{
    LCD_ST7789_RTS_1;
    delay_ms(100);
    LCD_ST7789_RTS_0;
    delay_ms(100);
    LCD_ST7789_RTS_1;
    delay_ms(100);
}

void LCD_ST7789_Init(void)
{
    gpio_function_en(LCD_ST7789_DC);
    gpio_output_en(LCD_ST7789_DC);
    gpio_input_dis(LCD_ST7789_DC);
    // gpio_function_en(LCD_ST7789_CN); // ???
    // gpio_output_en(LCD_ST7789_CN);
    // gpio_input_dis(LCD_ST7789_CN);
    gpio_function_en(LCD_ST7789_RTS);
    gpio_output_en(LCD_ST7789_RTS);
    gpio_input_dis(LCD_ST7789_RTS);
    // gpio_function_en(LCD_ST7789_BL); //BL not supported due to limited pin count
    // gpio_output_en(LCD_ST7789_BL);   //BL shall be connected to VCC
    // gpio_input_dis(LCD_ST7789_BL);

    // LCD_ST7789_BL_1;
    // LCD_ST7789_CN_0;

    spi_master_init(SPI_MODULE, sys_clk.pll_clk * 1000000 / SPI_CLK_SPEED, SPI_MODE0);

    gspi_set_pin(&gspi_pin_config);
    spi_master_config_plus(SPI_MODULE, &spi_b91m_slave_protocol_config);
}

void LCD_ST7789_InitReg(void)
{
    LCD_ST7789_Reset();
    LCD_ST7789_SendCommand(0x36);
    LCD_ST7789_SendByte(0x00);

    LCD_ST7789_SendCommand(0x3A);
    LCD_ST7789_SendByte(0x05);

    LCD_ST7789_SendCommand(0x21);

    LCD_ST7789_SendCommand(0x2A);
    LCD_ST7789_SendByte(0x00);
    LCD_ST7789_SendByte(0x00);
    LCD_ST7789_SendByte(0x01);
    LCD_ST7789_SendByte(0x3F);

    LCD_ST7789_SendCommand(0x2B);
    LCD_ST7789_SendByte(0x00);
    LCD_ST7789_SendByte(0x00);
    LCD_ST7789_SendByte(0x00);
    LCD_ST7789_SendByte(0xEF);

    LCD_ST7789_SendCommand(0xB2);
    LCD_ST7789_SendByte(0x0C);
    LCD_ST7789_SendByte(0x0C);
    LCD_ST7789_SendByte(0x00);
    LCD_ST7789_SendByte(0x33);
    LCD_ST7789_SendByte(0x33);

    LCD_ST7789_SendCommand(0xB7);
    LCD_ST7789_SendByte(0x35);

    LCD_ST7789_SendCommand(0xBB);
    LCD_ST7789_SendByte(0x1F);

    LCD_ST7789_SendCommand(0xC0);
    LCD_ST7789_SendByte(0x2C);

    LCD_ST7789_SendCommand(0xC2);
    LCD_ST7789_SendByte(0x01);

    LCD_ST7789_SendCommand(0xC3);
    LCD_ST7789_SendByte(0x12);

    LCD_ST7789_SendCommand(0xC4);
    LCD_ST7789_SendByte(0x20);

    LCD_ST7789_SendCommand(0xC6);
    LCD_ST7789_SendByte(0x0F);

    LCD_ST7789_SendCommand(0xD0);
    LCD_ST7789_SendByte(0xA4);
    LCD_ST7789_SendByte(0xA1);

    LCD_ST7789_SendCommand(0xE0);
    LCD_ST7789_SendByte(0xD0);
    LCD_ST7789_SendByte(0x08);
    LCD_ST7789_SendByte(0x11);
    LCD_ST7789_SendByte(0x08);
    LCD_ST7789_SendByte(0x0C);
    LCD_ST7789_SendByte(0x15);
    LCD_ST7789_SendByte(0x39);
    LCD_ST7789_SendByte(0x33);
    LCD_ST7789_SendByte(0x50);
    LCD_ST7789_SendByte(0x36);
    LCD_ST7789_SendByte(0x13);
    LCD_ST7789_SendByte(0x14);
    LCD_ST7789_SendByte(0x29);
    LCD_ST7789_SendByte(0x2D);

    LCD_ST7789_SendCommand(0xE1);
    LCD_ST7789_SendByte(0xD0);
    LCD_ST7789_SendByte(0x08);
    LCD_ST7789_SendByte(0x10);
    LCD_ST7789_SendByte(0x08);
    LCD_ST7789_SendByte(0x06);
    LCD_ST7789_SendByte(0x06);
    LCD_ST7789_SendByte(0x39);
    LCD_ST7789_SendByte(0x44);
    LCD_ST7789_SendByte(0x51);
    LCD_ST7789_SendByte(0x0B);
    LCD_ST7789_SendByte(0x16);
    LCD_ST7789_SendByte(0x14);
    LCD_ST7789_SendByte(0x2F);
    LCD_ST7789_SendByte(0x31);
    LCD_ST7789_SendCommand(0x21);

    LCD_ST7789_SendCommand(0x11);
    delay_ms(120);
    LCD_ST7789_SendCommand(0x29);
}

void LCD_ST7789_SetAttributes(uint32_t Scan_dir)
{
    // Get the screen scan direction
    LCD_ST7789_ST.SCAN_DIR  = Scan_dir;
    uint8_t MemoryAccessReg = 0x00;
    // Get GRAM and LCD width and height
    if (Scan_dir == HORIZONTAL) {
        LCD_ST7789_ST.HEIGHT = LCD_ST7789_WIDTH;
        LCD_ST7789_ST.WIDTH  = LCD_ST7789_HEIGHT;
        MemoryAccessReg      = 0X70;
    } else {
        LCD_ST7789_ST.HEIGHT = LCD_ST7789_HEIGHT;
        LCD_ST7789_ST.WIDTH  = LCD_ST7789_WIDTH;
        MemoryAccessReg      = 0X00;
    }
    // Set the read / write scan direction of the frame memory
    LCD_ST7789_SendCommand(0x36);         // MX, MY, RGB mode
    LCD_ST7789_SendByte(MemoryAccessReg); // 0x08 set RGB
}

void LCD_ST7789_SetWindows(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend)
{
    if (LCD_ST7789_ST.SCAN_DIR == VERTICAL) {
        // set the X coordinates
        LCD_ST7789_SendCommand(0x2A);
        LCD_ST7789_SendByte(Xstart >> 8);
        LCD_ST7789_SendByte(Xstart & 0xff);
        LCD_ST7789_SendByte((Xend - 1) >> 8);
        LCD_ST7789_SendByte((Xend - 1) & 0xff);
        // set the Y coordinates
        LCD_ST7789_SendCommand(0x2B);
        LCD_ST7789_SendByte((Ystart + 0) >> 8);
        LCD_ST7789_SendByte(Ystart + 0);
        LCD_ST7789_SendByte((Yend + 0 - 1) >> 8);
        LCD_ST7789_SendByte(Yend + 0 - 1);
    } else {
        // set the X coordinates
        LCD_ST7789_SendCommand(0x2A);
        LCD_ST7789_SendByte((Xstart + 0) >> 8);
        LCD_ST7789_SendByte(Xstart + 0);
        LCD_ST7789_SendByte((Xend + 0 - 1) >> 8);
        LCD_ST7789_SendByte(Xend + 0 - 1);
        // set the Y coordinates
        LCD_ST7789_SendCommand(0x2B);
        LCD_ST7789_SendByte(Ystart >> 8);
        LCD_ST7789_SendByte(Ystart);
        LCD_ST7789_SendByte((Yend - 1) >> 8);
        LCD_ST7789_SendByte(Yend - 1);
    }
    LCD_ST7789_SendCommand(0X2C);
}

void LCD_2IN_SetCursor(uint16_t X, uint16_t Y)
{
    LCD_ST7789_SendCommand(0x2a);
    LCD_ST7789_SendByte(X >> 8);
    LCD_ST7789_SendByte(X);
    LCD_ST7789_SendByte(X >> 8);
    LCD_ST7789_SendByte(X);
    LCD_ST7789_SendCommand(0x2b);
    LCD_ST7789_SendByte(Y >> 8);
    LCD_ST7789_SendByte(Y);
    LCD_ST7789_SendByte(Y >> 8);
    LCD_ST7789_SendByte(Y);
    LCD_ST7789_SendCommand(0x2C);
}

void LCD_2IN_WriteData_Word(uint16_t data)
{
#if 0
    LCD_ST7789_SendByte((data >> 8) & 0xFF);
    LCD_ST7789_SendByte((data & 0xFF));
#else
    uint8_t wdata[2] = {(data >> 8) & 0xFF, (data & 0xFF)};
    LCD_ST7789_DC_1;
    spi_master_write_plus(SPI_MODULE, SPI_WRITE_DATA_SINGLE_CMD, (uint32_t)NULL, (uint8_t *)&wdata, 2, SPI_MODE_WR_WRITE_ONLY);
#endif
}

void LCD_2IN_DrawPaint(uint16_t x, uint16_t y, uint16_t Color)
{
    LCD_2IN_SetCursor(x, y);
    LCD_2IN_WriteData_Word(Color);
}

void LCD_Init(void)
{
    LCD_ST7789_Init();
    LCD_ST7789_SetAttributes(VERTICAL);
    LCD_ST7789_InitReg();
    LCD_Fill(BACK_COLOR);
}

uint8_t g_lcd_buff[LCD_ST7789_WIDTH * LCD_ST7789_HEIGHT * 2];

void LCD_Fill(uint16_t color)
{
    LCD_ST7789_SetWindows(0, 0, LCD_ST7789_ST.WIDTH, LCD_ST7789_ST.HEIGHT);
#if 1
    for (uint32_t i = 0; i < LCD_ST7789_WIDTH; i++) {
        for (uint32_t j = 0; j < LCD_ST7789_HEIGHT; j++) {
            LCD_ST7789_SendByte((color >> 8) & 0xFF);
            LCD_ST7789_SendByte((color & 0xFF));
        }
    }
#else
    for (uint32_t i = 0; i < LCD_ST7789_WIDTH; i++) {
        for (uint32_t j = 0; j < LCD_ST7789_HEIGHT; j++) {
            uint32_t index        = (i * LCD_ST7789_HEIGHT + j) * 2;
            g_lcd_buff[index]     = (color >> 8) & 0xFF;
            g_lcd_buff[index + 1] = (color & 0xFF);
        }
    }
    LCD_ST7789_SendBytes(g_lcd_buff, LCD_ST7789_WIDTH * LCD_ST7789_HEIGHT * 2);
#endif
}

void LCD_ClearWindow(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, uint16_t color)
{
    uint16_t i, j;
    LCD_ST7789_SetWindows(Xstart, Ystart, Xend, Yend);
    for (i = Ystart; i <= Yend; i++) {
        for (j = Xstart; j <= Xend; j++) {
            LCD_2IN_WriteData_Word(color);
        }
    }
}

void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t size, uint8_t mode)
{
    uint8_t  temp, t1, t;
    uint16_t y0    = y;
    uint8_t  csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);
    num            = num - ' ';

    for (t = 0; t < csize; t++) {
        if (size == 12) {
            temp = asc2_1206[num][t];
        } else if (size == 16) {
            temp = asc2_1608[num][t];
        } else if (size == 24) {
            temp = asc2_2412[num][t];
        } else {
            return;
        }

        for (t1 = 0; t1 < 8; t1++) {
            if (temp & 0x80) {
                LCD_2IN_DrawPaint(x, y, POINT_COLOR);
            } else if (mode == 0) {
                LCD_2IN_DrawPaint(x, y, BACK_COLOR);
            }

            temp <<= 1;
            y++;

            if (y >= LCD_ST7789_ST.HEIGHT) {
                return;
            }

            if ((y - y0) == size) {
                y = y0;
                x++;

                if (x >= LCD_ST7789_ST.WIDTH) {
                    return;
                }

                break;
            }
        }
    }
}

uint32_t LCD_Pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;

    while (n--) {
        result *= m;
    }

    return result;
}

void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size)
{
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++) {
        temp = (num / LCD_Pow(10, len - t - 1)) % 10;

        if (enshow == 0 && t < (len - 1)) {
            if (temp == 0) {
                LCD_ShowChar(x + (size / 2) * t, y, ' ', size, 0);
                continue;
            } else {
                enshow = 1;
            }
        }

        LCD_ShowChar(x + (size / 2) * t, y, temp + '0', size, 0);
    }
}

void LCD_ShowxNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode)
{
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++) {
        temp = (num / LCD_Pow(10, len - t - 1)) % 10;

        if (enshow == 0 && t < (len - 1)) {
            if (temp == 0) {
                if (mode & 0X80) {
                    LCD_ShowChar(x + (size / 2) * t, y, '0', size, mode & 0X01);
                } else {
                    LCD_ShowChar(x + (size / 2) * t, y, ' ', size, mode & 0X01);
                }

                continue;
            } else {
                enshow = 1;
            }
        }

        LCD_ShowChar(x + (size / 2) * t, y, temp + '0', size, mode & 0X01);
    }
}

void LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint8_t *p)
{
    uint8_t x0 = x;
    width += x;
    height += y;

    while ((*p <= '~') && (*p >= ' ')) {
        if (x >= width) {
            x = x0;
            y += size;
        }

        if (y >= height) {
            break;
        }

        LCD_ShowChar(x, y, *p, size, 0);
        x += size / 2;
        p++;
    }
}
#define LCD_LINE_SPACING          (24)
#define LCD_FIRST_LINE_OFFSET     (10)
#define LCD_LINE_LENGTH           (18)
#define LCD_ACCEPTABLE_PART_LEN    (3)

void LCD_showLine(uint8_t line, char * msg) {
    LCD_ShowString(20, (line * LCD_LINE_SPACING + LCD_FIRST_LINE_OFFSET), 240 - 20, 24, 24, msg);
}

void LCD_showText(char *text) {
    char line_buffer[LCD_LINE_LENGTH +1];
    uint16_t index = 0;
    uint16_t forwardIndex;
    uint16_t length;
    uint16_t nextWordLen;
    uint16_t outoutFillLevel = 0;
    uint16_t lineNumber = 0;


    length = strlen(text);

    while (index < length) {
        // calculated next word's length
        nextWordLen = 0;
        forwardIndex = index;
        while(forwardIndex < length) {
            if((text[forwardIndex] ==' ') || (text[forwardIndex] == 0x00)) break;
            nextWordLen++;
            forwardIndex ++;
        }

        // nothing more to process
        if(nextWordLen == 0) break;

        if(outoutFillLevel + nextWordLen > LCD_LINE_LENGTH) { // we need to move to new line
            if(nextWordLen <= LCD_LINE_LENGTH) { // word can fit into display, let's make a newline
                line_buffer[outoutFillLevel] = 0x00;
                LCD_showLine(lineNumber++, line_buffer);
                memcpy(line_buffer, &text[index], nextWordLen);
                index = index + nextWordLen + 1;
                outoutFillLevel = nextWordLen;

                // new word expected?
                if (outoutFillLevel < LCD_LINE_LENGTH) {
                    line_buffer[outoutFillLevel++] = ' ';
                }
                continue;
            }
            else { // word is longer than a line
                if(LCD_LINE_LENGTH - outoutFillLevel >= LCD_ACCEPTABLE_PART_LEN) { // will if be elegant to start in current line?
                    memcpy(line_buffer + outoutFillLevel, &text[index], LCD_LINE_LENGTH - outoutFillLevel);
                    index = index + LCD_LINE_LENGTH - outoutFillLevel;
                    outoutFillLevel = LCD_LINE_LENGTH;
                    line_buffer[outoutFillLevel] = 0x00;
                    LCD_showLine(lineNumber++, line_buffer);
                    outoutFillLevel = 0;
                    continue;
                } else { // no it won't. start from a new line
                    line_buffer[outoutFillLevel] = 0x00;
                    LCD_showLine(lineNumber++, line_buffer);
                    memcpy(line_buffer, &text[index], LCD_LINE_LENGTH);
                    line_buffer[LCD_LINE_LENGTH] = 0x00;
                    index = index + LCD_LINE_LENGTH;
                    LCD_showLine(lineNumber++, line_buffer);
                    outoutFillLevel = 0;
                    continue;
                }
            }
        } else { // no need for new line
            memcpy(&line_buffer[outoutFillLevel], &text[index], nextWordLen);
            outoutFillLevel = outoutFillLevel + nextWordLen;
            line_buffer[outoutFillLevel] = ' ';
            outoutFillLevel ++;
            index = index + nextWordLen + 1;
        }
    }
    // display last line
    if(outoutFillLevel != 0) {
        line_buffer[outoutFillLevel] = 0x00;
        LCD_showLine(lineNumber, line_buffer);
    }
}

#endif