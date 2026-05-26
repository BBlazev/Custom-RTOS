#include <stdint.h>
#include <stddef.h>
#include "sem.h"
#include "task.h"

void sem_init(sem_t *s, int32_t init_count)
{
    s->count = init_count;
    s->waiters = NULL;
}

void sem_take(sem_t *s)
{
    uint32_t primask = enter_primask();

    if(s->count > 0)
    {
        s->count--;
        exit_primask(primask);
        return;
    }

    block_current_task(&s->waiters);
    exit_primask(primask);
}

void sem_give(sem_t *s)
{
    uint32_t primask = enter_primask();

    if(s->waiters != NULL)
    {
        TCB_t *woken = unblock_one_task(&s->waiters);
        (void)woken;

        rtos_schedule();
    }
    else
        s->count++;

    exit_primask(primask);
}