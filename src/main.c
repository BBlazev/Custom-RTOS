#include <stdint.h>
#include "uart.h"
#include "systick.h"

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
    uart_init();
    uart_puts("RTOS booting\r\n");
    systick_init(1);
    uint32_t last_printed = 0;
    for(;;){
        uint32_t now = systick_get_ticks();
        if(now - last_printed >= 1000){

            uart_puts("tick=");
            uart_put_uint(now);
            uart_puts("\r\n");
            last_printed = now;
        }
    }

    return 0;
}