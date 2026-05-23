#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <stddef.h>

#define TASK_STACK_WORDS    256
#define MAX_TASKS           4
#define MAX_PRIO            4

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED
} task_state_t;

typedef void (*task_entry_t)(void);

typedef struct tcb {
    uint32_t        *sp;
    uint32_t        stack[TASK_STACK_WORDS];
    task_state_t    state;
    uint32_t        id;
    uint32_t        priority;
    uint32_t        wake_at;
    struct tcb      *next;
} TCB_t;

extern TCB_t *g_current_task;
extern TCB_t *g_next_task;

int  task_create(task_entry_t entry, uint32_t id, uint32_t priority);
void rtos_yield(void);
void rtos_start(void);
void rtos_sleep(uint32_t ms);
void rtos_schedule(void);
void rtos_tick(uint32_t now);

void block_current_task(TCB_t **wait_list);
TCB_t *unblock_one_task(TCB_t **wait_list);

#endif