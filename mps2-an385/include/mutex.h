#ifndef MUTEX_H
#define MUTEX_H

#include <stdint.h>
#include "task.h"

typedef struct 
{
    TCB_t *owner;
    TCB_t *waiters;
    uint32_t owner_original_prio;
} mutex_t;

void mutex_init(mutex_t *m);
void mutex_lock(mutex_t *m);
void mutex_unlock(mutex_t *m);




#endif