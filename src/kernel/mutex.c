#include <stdint.h>
#include <stddef.h>
#include "mutex.h"
#include "task.h"

void mutex_init(mutex_t *m)
{
    m->owner = NULL;
    m->waiters = NULL;
    m->owner_original_prio = 0;

}

void mutex_lock(mutex_t *m)
{
    uint32_t primask = enter_primask();

    //if free, take it, remember prio so unlock can restore it
    if(m->owner == NULL)
    {
        m->owner = g_current_task;
        m->owner_original_prio = g_current_task->priority;
        exit_primask(primask);
        return;
    }

    if(g_current_task->priority < m->owner->priority)
        m->owner->priority = g_current_task->priority;

    block_current_task(&m->waiters);
    exit_primask(primask);

}

void mutex_unlock(mutex_t *m)
{
    uint32_t primask = enter_primask();

    if(m->owner != g_current_task)
    {
        exit_primask(primask);
        return;
    }

    g_current_task->priority = m->owner_original_prio;

    if(m->waiters != NULL)
    {
        TCB_t *next = unblock_one_task(&m->waiters);
        m->owner = next;
        m->owner_original_prio = next->priority;
        rtos_schedule();
    }
    else 
        m->owner = NULL;

    exit_primask(primask);
}