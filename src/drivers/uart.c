#include <stdint.h>
#include "uart.h"

#define UART_BASE   0X40004000U

#define UART0_DATA  (*(volatile uint32_t *)(UART_BASE + 0x000))
#define UART0_STATE (*(volatile uint32_t *)(UART_BASE + 0x004))
#define UART0_CTRL  (*(volatile uint32_t *)(UART_BASE + 0x008))

#define UART_STATE_TX_FULL  (1U << 0)
#define UART_CTRL_TX_EN     (1U << 0)
#define UART_CTRL_RX_EN     (1U << 1)

void uart_init(void)
{
    UART0_CTRL = UART_CTRL_TX_EN | UART_CTRL_RX_EN;
}

void uart_putc(char c)
{
    while (UART0_STATE & UART_STATE_TX_FULL) {}
    UART0_DATA = (uint32_t)c;
}

void uart_puts(const char* s)
{
    while (*s)
    {
        uart_putc(*s++);
    }
    
}