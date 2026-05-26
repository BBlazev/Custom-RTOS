#include <stdint.h>
#include "uart.h"
#include "task.h"
#include "systick.h"
#include "sem.h"
#include "mutex.h"

#define SHPR3_PENDSV    (*(volatile uint8_t*)0xE000ED22)
#define SHPR3_SYSTICK   (*(volatile uint8_t*)0xE000ED23)

typedef struct 
{
    uint32_t last_reading;
    uint32_t latest_processed;
    uint32_t reading_count;
}sensor_data_t;

static sensor_data_t g_data;
static sem_t g_sem;
static mutex_t g_mtx;
static uint32_t g_pending_reading;

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

static uint32_t rng_state = 12345;
static uint32_t fake_sensor_read(void)
{
    rng_state = rng_state * 1103515245u + 12345u;
    return (rng_state >> 16) & 0x3FF;
}

static void sensor_task(void)
{
    for(;;)
    {
        rtos_sleep(1000);
        g_pending_reading = fake_sensor_read();
        sem_give(&g_sem);
    }
}

static void processor_task(void)
{
    for(;;)
    {
        sem_take(&g_sem);
        uint32_t raw = g_pending_reading;
        uint32_t processed = raw / 2;

        mutex_lock(&g_mtx);
        g_data.last_reading = raw;
        g_data.latest_processed = processed;
        g_data.reading_count++;
        mutex_unlock(&g_mtx);
    }
}

static void logger(void)
{
    for(;;)
    {
        rtos_sleep(1000);
        mutex_lock(&g_mtx);
        uint32_t r = g_data.last_reading;
        uint32_t p = g_data.latest_processed;
        uint32_t n = g_data.reading_count;
        mutex_unlock(&g_mtx);
        uart_puts("[t=");
        uart_put_uint(systick_get_ticks());
        uart_puts("ms] sensor=");
        uart_put_uint(r);
        uart_puts(" processed=");
        uart_put_uint(p);
        uart_puts(" (readings: ");
        uart_put_uint(n);
        uart_puts(")\r\n");
    }
}

int main(void)
{

    __asm volatile("cpsid i"); //disable interrupt until we are rdy

    uart_init();
    uart_puts("RTOS booting\r\n");

    SHPR3_PENDSV  = 0xFF;
    SHPR3_SYSTICK = 0xE0;

    g_data.last_reading = 0;
    g_data.latest_processed = 0;
    g_data.reading_count = 0;
    mutex_init(&g_mtx);
    sem_init(&g_sem, 0);

    task_create(sensor_task, 1, 0);  
    task_create(processor_task, 2, 1);
    task_create(logger, 3, 2);

    systick_init(1);

    uart_puts("System running\r\n");
    rtos_start();
    for(;;){}
}