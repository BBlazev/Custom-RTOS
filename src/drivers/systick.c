#include <stdint.h>
#include <systick.h>
#include "task.h"


#define SYSTICK_BASE 0XE000E010U
#define SYSTICK_CTRL (*(volatile uint32_t *)(SYSTICK_BASE + 0x00))
#define SYSTICK_LOAD (*(volatile uint32_t *)(SYSTICK_BASE + 0x04))
#define SYSTICK_VAL  (*(volatile uint32_t *)(SYSTICK_BASE + 0x08))

#define SYSTICK_CTRL_ENABLE     (1U << 0)
#define SYSTICK_CTRL_TICKINT    (1U << 1)
#define SYSTICK_CTRL_CLKSOURCE  (1U << 2)

#define CPU_HZ 25000000U

static volatile uint32_t g_ticks = 0;


void systick_init(uint32_t ms)
{
    SYSTICK_LOAD = (CPU_HZ / 1000U) * ms -1U;
    SYSTICK_VAL = 0U;
    SYSTICK_CTRL = SYSTICK_CTRL_ENABLE | SYSTICK_CTRL_TICKINT | SYSTICK_CTRL_CLKSOURCE;
}

uint32_t systick_get_ticks(void)
{
    return g_ticks;
}

//slot 15 in vector table
void SysTick_Handler(void)
{
    g_ticks++;
    rtos_tick(g_ticks);
}