#ifndef SEM_H
#define SEM_H

#include <stdint.h>
#include "task.h"

typedef struct {
    int32_t count;
    TCB_t *waiters;
} sem_t;

void sem_init(sem_t *s, int32_t init_count);
void sem_give(sem_t *s);
void sem_take(sem_t *s);


#endif