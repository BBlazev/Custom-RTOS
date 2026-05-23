#include <stdint.h>
#include <stddef.h>
#include "task.h"
#include "systick.h"

#define SCB_ISCR            (*(volatile uint32_t *)0xE000ED04)
#define SCB_ISCR_PENDSVSET  (1U << 28)

static TCB_t   *g_ready[MAX_PRIO];
static TCB_t   *g_blocked;
static TCB_t   *g_wait_list;
static TCB_t    g_tasks[MAX_TASKS];
static uint32_t g_num_tasks = 0;

 
TCB_t *g_current_task = NULL;  
TCB_t *g_next_task    = NULL;

static void idle_task(void) { for(;;){} }

static void append_to_ready(TCB_t *t)
{
    t->next = NULL;
    if (g_ready[t->priority] == NULL) {
        g_ready[t->priority] = t;
        return;
    }
    TCB_t *p = g_ready[t->priority];
    while (p->next != NULL)
        p = p->next;
    p->next = t;
}

static void remove_from_ready(TCB_t *t)
{
    TCB_t **pp = &g_ready[t->priority];
    while (*pp != NULL && *pp != t) {
        pp = &(*pp)->next;
    }
    if (*pp == t) {
        *pp = t->next;
    }
    t->next = NULL;
}

static void insert_blocked(TCB_t *t)
{
    TCB_t **pp = &g_blocked;
    while (*pp != NULL && (*pp)->wake_at <= t->wake_at) {
        pp = &(*pp)->next;
    }
    t->next = *pp;
    *pp = t;
}

int task_create(task_entry_t entry, uint32_t id, uint32_t priority)
{
    if (g_num_tasks >= MAX_TASKS)
        return -1;
    if(priority >= MAX_PRIO) 
        return -1;

    TCB_t *t = &g_tasks[g_num_tasks];
    t->state = TASK_READY;
    t->id = id;
    t->priority = priority;
    t->wake_at = 0;
    t->next = NULL;

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

    append_to_ready(t);
    g_num_tasks++;
    return 0;
}
// RR
static void scheduler(void)
{
    for(uint32_t p = 0; p < MAX_PRIO; p++)
    {
        if(g_ready[p] != 0)
        {
            g_next_task = g_ready[p];
            g_ready[p] = g_ready[p]->next;
            g_next_task->next = NULL;
            append_to_ready(g_next_task);
            return;
        }
    }
    g_next_task = NULL;

}

void rtos_yield(void)
{
    rtos_schedule();
}

void rtos_start(void)
{
    task_create(idle_task, 0, MAX_PRIO-1);

    scheduler();
    g_current_task = NULL; 

    SCB_ISCR = SCB_ISCR_PENDSVSET; // pend pendsv, will fire as soon as interrupts run

    __asm volatile("cpsie i"); // enable interrupts, pendsv will fire immediately

    for(;;){} // just a guard, never reaches
}

void rtos_schedule(void)
{
    scheduler();
    SCB_ISCR = SCB_ISCR_PENDSVSET;
}

void rtos_sleep(uint32_t ms)
{
    __asm volatile("cpsid i");
    g_current_task->wake_at = systick_get_ticks() + ms;
    g_current_task->state = TASK_BLOCKED;

    remove_from_ready(g_current_task);
    insert_blocked(g_current_task);
    scheduler();

    SCB_ISCR = SCB_ISCR_PENDSVSET;

    __asm volatile("cpsie i");
}

void rtos_tick(uint32_t now)
{
    while (g_blocked != NULL && g_blocked->wake_at <= now) {
        TCB_t *t = g_blocked;
        g_blocked = g_blocked->next;   
        t->state = TASK_READY;
        append_to_ready(t);            
    }

    scheduler();
    SCB_ISCR = SCB_ISCR_PENDSVSET;
}

void block_current_task(TCB_t **wait_list)
{
    TCB_t *t = g_current_task;
    t->state = TASK_BLOCKED;

    remove_from_ready(t);

    //append it to wait list
    t->next = NULL;
    if(*wait_list == NULL) //if empty, put it as head
        *wait_list = t;
    else
    {
        TCB_t *p = *wait_list;
        while(p->next != NULL) //traverse LL, put t as last
            p = p->next;
        p->next = t;
    }
    scheduler();
    SCB_ISCR = SCB_ISCR_PENDSVSET;
    
}

TCB_t *unblock_one_task(TCB_t **wait_list)
{
    if(*wait_list == NULL) 
        return NULL;

    //t will be head from wait list, t->next is now head of wait list
    TCB_t *t = *wait_list;
    *wait_list = t->next;
    t->next = NULL;

    t->state = TASK_READY;
    append_to_ready(t);

    return t;
}