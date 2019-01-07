#ifndef OPERATIVE_SISTEM_TBARRIER_H
#define OPERATIVE_SISTEM_TBARRIER_H

#include "tqueue.h"

typedef struct {
    TQueue waiting_list;
    unsigned count;
    unsigned barrier_size;
} bthread_barrier_t;

typedef struct {
} bthread_barrierattr_t;

int bthread_barrier_init(bthread_barrier_t *b, const bthread_barrierattr_t *attr, unsigned count);

int bthread_barrier_destroy(bthread_barrier_t *b);

int bthread_barrier_wait(bthread_barrier_t *b);


#endif //OPERATIVE_SISTEM_TBARRIER_H
