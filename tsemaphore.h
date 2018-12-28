#ifndef OPERATIVE_SISTEM_TSEMAPHORE_H
#define OPERATIVE_SISTEM_TSEMAPHORE_H

#include <stdlib.h>
#include <assert.h>
#include "tqueue.h"
#include "bthread_private.h"

typedef struct {
    int value;
    TQueue waiting_list;
} bthread_sem_t;

// pshared is ignored, defined for compatibility with pthread
int bthread_sem_init(bthread_sem_t *m, int pshared, int value);

int bthread_sem_destroy(bthread_sem_t *m);

int bthread_sem_wait(bthread_sem_t *m);

int bthread_sem_post(bthread_sem_t *m);

#define bthread_sem_up(s) \
 bthread_sem_post(s);

#define bthread_sem_down(s) \
 bthread_sem_wait(s);


#endif //OPERATIVE_SISTEM_TSEMAPHORE_H
