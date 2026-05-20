#include <stdint.h>
#include <stddef.h>
#include "task.h"

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

    //SAVE FINAL SP
    t->sp = sp;

    g_num_tasks++;
    return 0;
}