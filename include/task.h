#ifndef TASK_H
#define TASK_H


#include <stdint.h>
#include <stddef.h>

#define TASK_STACK_WORDS    256
#define MAX_TASKS           4

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED
} task_state_t;

typedef void (*task_entry_t)(void);

typedef struct {
    uint32_t *sp;
    uint32_t stack[TASK_STACK_WORDS];
    task_state_t state;
    uint32_t id;
} TCB_t;

int task_create(task_entry_t entry, uint32_t id);
void rtos_yield(void);
void rtos_start(void);
void rtos_schedule(void);

#endif