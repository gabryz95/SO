#ifndef OPERATIVE_SISTEM_TMUTEX_H
#define OPERATIVE_SISTEM_TMUTEX_H

#include <assert.h>
#include "tqueue.h"
#include "bthread_private.h"

typedef struct {
    void *owner;
    TQueue waiting_list;
} bthread_mutex_t;

typedef struct {
} bthread_mutexattr_t;

int bthread_mutex_init(bthread_mutex_t *m, const bthread_mutexattr_t *attr);

int bthread_mutex_destroy(bthread_mutex_t *m);

int bthread_mutex_lock(bthread_mutex_t *m);

int bthread_mutex_trylock(bthread_mutex_t *m);

int bthread_mutex_unlock(bthread_mutex_t *m);


#endif //OPERATIVE_SISTEM_TMUTEX_H
