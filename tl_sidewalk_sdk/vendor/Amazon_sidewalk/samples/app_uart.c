/********************************************************************************************************
 * @file    app_uart.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2025
 *
 * @par     Copyright (c) 2022, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#if (FREERTOS_ENABLE)
    #include "tlk_riscv.h"
    #include <FreeRTOS.h>
    #include <task.h>
    #include "app_freertos.h"
#endif
#include "app.h"
#include "app_uart.h"

#if config_HW_SELECT
#define UART0_TX_PIN GPIO_FC_PE4
#define UART0_RX_PIN GPIO_FC_PE5
#else
#define UART0_TX_PIN GPIO_FC_PA1
#define UART0_RX_PIN GPIO_FC_PA2
#endif
#define APP_UART_BAUDRATE        1000000
#define UART_MODULE_SEL         UART0

#define APP_UART_RX_BUF_SIZE     256

typedef struct {
    u8 buf[APP_UART_RX_BUF_SIZE];
    u16 head;
    u16 tail;
} uart_ringbuf_t;

static uart_ringbuf_t uart_rx_rb __attribute__((aligned(4))) ={0};

_attribute_ram_code_sec_ int app_uart_ringbuf_put(u8 ch)
{
    u16 next_tail = (uart_rx_rb.tail + 1) % APP_UART_RX_BUF_SIZE;

    if(next_tail == uart_rx_rb.head) {
        return -1;
    }
    uart_rx_rb.buf[uart_rx_rb.tail] = ch;
    uart_rx_rb.tail = next_tail;
    return 0;
}

int app_uart_ringbuf_get(u8 *pch)
{
    if(uart_rx_rb.head == uart_rx_rb.tail) {
        return -1;
    }
    *pch = uart_rx_rb.buf[uart_rx_rb.head];
    uart_rx_rb.head = (uart_rx_rb.head + 1) % APP_UART_RX_BUF_SIZE;
    return 0;
}

u16 app_uart_ringbuf_len(void)
{
    return (uart_rx_rb.tail - uart_rx_rb.head + APP_UART_RX_BUF_SIZE) % APP_UART_RX_BUF_SIZE;
}

u8 tlk_getchar()
{
    u8 ch = 0xFF;
    app_uart_ringbuf_get(&ch);
    return ch;
}

int putchar(int ch)
{
    return uart_send_byte(UART0, ch);
}

int    fflush (FILE *) {
#if (TLKAPI_DEBUG_ENABLE)
    while(tlkapi_debug_isBusy()) {
        tlkapi_debug_handler();
    }
#endif
}

void app_uart_init(void)
{
    //init the uart to adapt cli functiuon
    unsigned short div  = 0;
    unsigned char  bwpc = 0;
    uart_hw_fsm_reset(UART0);
    uart_set_pin(UART0, UART0_TX_PIN, UART0_RX_PIN);
    uart_cal_div_and_bwpc(APP_UART_BAUDRATE, sys_clk.pclk * 1000 * 1000, &div, &bwpc);
    uart_init(UART0, div, bwpc, UART_PARITY_NONE, UART_STOP_BIT_ONE);
    uart_clr_irq_mask(UART0, UART_RX_IRQ_MASK | UART_TX_IRQ_MASK | UART_TXDONE_MASK | UART_RXDONE_MASK);
    uart_clr_irq_status(UART_MODULE_SEL, UART_TXDONE_IRQ_STATUS);
    uart_set_irq_mask(UART0, UART_RXDONE_MASK);
    uart_set_rx_timeout_with_exp(UART0, bwpc, 12, UART_BW_MUL2, 0);
    plic_interrupt_enable(IRQ_UART0);
}

_attribute_ram_code_sec_ void uart0_irq_handler(void)
{
    if (uart_get_irq_status(UART_MODULE_SEL, UART_RX_ERR)) {
        uart_clr_irq_status(UART_MODULE_SEL, UART_RXBUF_IRQ_STATUS);
    }
    if (uart_get_irq_status(UART_MODULE_SEL, UART_RXBUF_IRQ_STATUS)) {
        unsigned char fifo_cnt = uart_get_rxfifo_num(UART_MODULE_SEL);
        for (int i = 0; i < fifo_cnt; i++) {
            app_uart_ringbuf_put(uart_read_byte(UART_MODULE_SEL));
        }
    }

    if (uart_get_irq_status(UART_MODULE_SEL, UART_RXDONE_IRQ_STATUS)) {
        unsigned char uart_fifo_cnt = uart_get_rxfifo_num(UART_MODULE_SEL);
        if (uart_fifo_cnt != 0) {
            for (int j = 0; j < uart_fifo_cnt; j++) {
                app_uart_ringbuf_put(uart_read_byte(UART_MODULE_SEL));
            }
        uart_clr_irq_status(UART_MODULE_SEL, UART_RXDONE_IRQ_STATUS);
        }
   }
}

#if (FREERTOS_ENABLE)
PLIC_ISR_REGISTER_OS(uart0_irq_handler, IRQ_UART0);
#else
PLIC_ISR_REGISTER(uart0_irq_handler, IRQ_UART0);
#endif

