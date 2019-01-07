#ifndef OPERATIVE_SISTEM_TCONDITION_H
#define OPERATIVE_SISTEM_TCONDITION_H

#include "tqueue.h"
#include "tmutex.h"

typedef struct {
    TQueue waiting_lits;
} bthread_cond_t;

typedef struct {
} bthread_condattr_t;

int bthread_cond_init(bthread_cond_t *c, const bthread_condattr_t *attr);

int bthread_cond_destroy(bthread_cond_t *c);

int bthread_cond_wait(bthread_cond_t *c, bthread_mutex_t *mutex);

int bthread_cond_signal(bthread_cond_t *c);

int bthread_cond_broadcast(bthread_cond_t *c);

#define bthread_cond_notify(s) \
bthread_cond_signal(s);

#define bthread_cond_notifyall(s) \
bthread_cond_broadcast(s);


#endif //OPERATIVE_SISTEM_TCONDITION_H
