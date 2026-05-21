#include <stdint.h>
#include <stddef.h>
#include "task.h"

#define SCB_ISCR            (*(volatile uint32_t *)0xE000ED04)
#define SCB_ISCR_PENDSVSET  (1U << 28)


static TCB_t g_tasks[MAX_TASKS];
static uint32_t g_num_tasks = 0;

TCB_t *g_current_task = NULL;
TCB_t *g_next_task = NULL;

int task_create(task_entry_t entry, uint32_t id)
{
    if (g_num_tasks >= MAX_TASKS)
        return -1;

    TCB_t *t = &g_tasks[g_num_tasks];
    t->state = TASK_READY;
    t->id = id;

    //push nowe 16 words by decrementing this pointer *sp
    uint32_t *sp = &t->stack[TASK_STACK_WORDS];

    //important: push from high adresses down,meaning xpsr is first
    //important: we decrement sp first, then dereference, starting at 255, 256 is empty,
    //           cortex m reason

    *(--sp) = 0x01000000U;      //xPSR "Thumb" bit from crotex m
    *(--sp) = (uint32_t)entry;  //PC
    *(--sp) = 0xFFFFFFFDU;       //LR EXC_RETURN
    *(--sp) = 0x00000000U;      //R12 
    *(--sp) = 0x00000000U;      //R3 
    *(--sp) = 0x00000000U;      //R2 
    *(--sp) = 0x00000000U;      //R1 
    *(--sp) = 0x00000000U;      //R0 

    //r4 - r11, manually
    *(--sp) = 0x00000000U;      //R11
    *(--sp) = 0x00000000U;      //R10
    *(--sp) = 0x00000000U;      //R9 
    *(--sp) = 0x00000000U;      //R8 
    *(--sp) = 0x00000000U;      //R7 
    *(--sp) = 0x00000000U;      //R6 
    *(--sp) = 0x00000000U;      //R5 
    *(--sp) = 0x00000000U;      //R4 

    //SAVE FINAL SP, points to 240
    t->sp = sp;

    g_num_tasks++;
    return 0;
}
// RR
static void pick_next_task(void)
{
    uint32_t current_idx = (uint32_t)(g_current_task - g_tasks);
    uint32_t next_idx = (current_idx + 1U) % g_num_tasks;
    g_next_task = &g_tasks[next_idx];
}

void rtos_yield(void)
{
    rtos_schedule();
}

void rtos_start(void)
{
    if(g_num_tasks == 0)
        return;

    g_next_task = &g_tasks[0];
    SCB_ISCR = SCB_ISCR_PENDSVSET; // pend pendsv, will fire as soon as interrupts run

    __asm volatile("cpsie i"); // enable interrupts, pendsv will fire immediately

    for(;;){} // just a guard, never reaches
}

void rtos_schedule(void)
{
    pick_next_task();
    SCB_ISCR = SCB_ISCR_PENDSVSET;
}