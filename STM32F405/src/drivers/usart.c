#include "usart.h"
#include <stdint.h>

#define USART2_BASE 0x40004400U

#define USART2_SR   (*(volatile uint32_t*)(USART2_BASE + 0x00))
#define USART_DR    (*(volatile uint32_t*)(USART2_BASE + 0x04))
#define USART_BRR   (*(volatile uint32_t*)(USART2_BASE + 0x08))
#define USART_CR    (*(volatile uint32_t*)(USART2_BASE + 0x0C))


