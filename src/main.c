#include <stdint.h>
#include "uart.h"
#include "task.h"
#include "systick.h"


#define SHPR3_PENDSV    (*(volatile uint8_t*)0xE000ED22)
#define SHPR3_SYSTICK   (*(volatile uint8_t*)0xE000ED23)


static uint32_t last_printed = 0;

static void print_timer(uint32_t ms, char c)
{
    for(;;)
    {
        uint32_t now = systick_get_ticks();
        if(now - last_printed >= ms)
        {
            uart_putc(c);
            last_printed = now;
        }

    }
}

static void task_a(void)
{
    for(;;) {
        uart_putc('H');
        rtos_sleep(10); 
    }
}

static void task_b(void)
{
    for(;;) {
        uart_putc('.');
    }
}

static void uart_put_uint(uint32_t v)
{
    char buf[11];
    int i = 10;
    buf[i--] = '\0';
    if (v == 0) {
        uart_putc('0');
        return;
    }
    while (v > 0 && i >= 0) {
        buf[i--] = '0' + (v % 10);
        v /= 10;
    }
    uart_puts(&buf[i + 1]);
}



int main(void)
{

    __asm volatile("cpsid i"); //disable interrupt until we are rdy

    uart_init();
    uart_puts("RTOS booting\r\n");

    
    SHPR3_PENDSV    = 0xFF;
    SHPR3_SYSTICK   = 0xE0;
    
    task_create(task_a, 1, 0);
    task_create(task_b, 2, 2);
    
    systick_init(1);

    uart_puts("Starting scheduler\r\n");
    rtos_start();
    for(;;){}

    return 0;
}