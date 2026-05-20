#include <stdint.h>
#include <stddef.h>
#include "task.h"

static TCB_t g_tasks[MAX_TASKS];
static uint32_t g_num_tasks = 0;

TCB_t *g_current_task = NULL;
TCB_t *g_next_task = NULL;

int task_create(task_entry_t entry, uint32_t id);