#include "uart.h"

int main(void)
{
    uart_init();
    uart_puts("RTOS booting on\r\n");
    
    for(;;){}

    return 0;
}