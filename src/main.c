#include <stdint.h>
#include "uart.h"
#include "task.h"
#include "systick.h"
#include "sem.h"
#include "mutex.h"

#define SHPR3_PENDSV    (*(volatile uint8_t*)0xE000ED22)
#define SHPR3_SYSTICK   (*(volatile uint8_t*)0xE000ED23)

static sem_t g_sem;
static mutex_t g_mtx;


static void print_timer(uint32_t ms, char c)
{
    uint32_t last_printed = 0;
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

static void busy_ms(uint32_t ms)
{
    uint32_t start = systick_get_ticks();
    while(systick_get_ticks() - start < ms){}
}


static void low_task(void)
{
    for(;;) {
        mutex_lock(&g_mtx);
        uart_putc('L');        
        busy_ms(50);           
        uart_putc('l');        
        mutex_unlock(&g_mtx);
        rtos_sleep(200);
    }
}

static void med_task(void)
{
    for(;;) {
        rtos_sleep(10);
        uart_putc('M');        
        busy_ms(30);
        rtos_sleep(100);
    }
}

static void high_task(void)
{
    for(;;) {
        rtos_sleep(20);        
        mutex_lock(&g_mtx);    
        uart_putc('H');        
        mutex_unlock(&g_mtx);
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
    
    sem_init(&g_sem, 0);
    mutex_init(&g_mtx);

    task_create(low_task, 1, 2);
    task_create(med_task, 2, 1);
    task_create(high_task, 3, 0);

    

    systick_init(1);

    uart_puts("Starting scheduler\r\n");
    rtos_start();
    for(;;){}

    return 0;
}