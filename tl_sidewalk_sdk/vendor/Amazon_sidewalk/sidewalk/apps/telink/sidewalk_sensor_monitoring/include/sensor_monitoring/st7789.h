/********************************************************************************************************
 * @file    st7789.h
 *
 * @brief   This is the header file for Telink RISC-V MCU
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
#ifndef ST7789_H
#define ST7789_H
/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C"
{
#endif
#include "tl_common.h"
#include "app_config.h"

#define WHITE      0xFFFF
#define BLACK      0x0000
#define BLUE       0x001F
#define BRED       0XF81F
#define GBLUE      0X07FF
#define RED        0xF800
#define MAGENTA    0xF81F
#define GREEN      0x07E0
#define CYAN       0x7FFF
#define YELLOW     0x1F3F
#define BROWN      0XBC40
#define BRRED      0XFC07
#define GRAY       0X8430
#define DARKBLUE   0X01CF
#define LIGHTBLUE  0X7D7C
#define GRAYBLUE   0X5458
#define LIGHTGREEN 0X841F
#define LGRAY      0XC618
#define LGRAYBLUE  0XA651
#define LBBLUE     0X2B12

// Additional GPIO lines related with LCD
#define LCD_ST7789_DC_0 gpio_set_low_level(LCD_ST7789_DC)
#define LCD_ST7789_DC_1 gpio_set_high_level(LCD_ST7789_DC)


// #define LCD_ST7789_CN GPIO_PD3
// #define LCD_ST7789_CN_0 gpio_set_low_level(LCD_ST7789_CN)
// #define LCD_ST7789_CN_1 gpio_set_high_level(LCD_ST7789_CN)

#define LCD_ST7789_RTS GPIO_PA1
#define LCD_ST7789_RTS_0 gpio_set_low_level(LCD_ST7789_RTS)
#define LCD_ST7789_RTS_1 gpio_set_high_level(LCD_ST7789_RTS)

#if USE_BL // Due to low pincount sidewalk demo does not support display blanking
#define LCD_ST7789_BL GPIO_PE2
#define LCD_ST7789_BL_0   gpio_set_low_level(LCD_ST7789_BL)
#define LCD_ST7789_BL_1   gpio_set_high_level(LCD_ST7789_BL)
#else
#define LCD_ST7789_BL_0
#define LCD_ST7789_BL_1
#endif

#define HORIZONTAL        0
#define VERTICAL          1
#define LCD_ST7789_HEIGHT 320
#define LCD_ST7789_WIDTH  240

    typedef struct
    {
        uint16_t WIDTH;
        uint16_t HEIGHT;
        uint32_t SCAN_DIR;
    } LCD_ST7789_ATTRIBUTES;

    extern LCD_ST7789_ATTRIBUTES LCD_ST7789_ST;

    extern uint16_t POINT_COLOR;
    extern uint16_t BACK_COLOR;
    void            LCD_Init(void);
    void            LCD_Fill(uint16_t color);
    void            LCD_2IN_DrawPaint(uint16_t x, uint16_t y, uint16_t Color);
    void            LCD_ClearWindow(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, uint16_t color);
    void            LCD_showText(char *text);

    void LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint8_t *p);

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
#endif
