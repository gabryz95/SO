#include <assert.h>
#include <stdlib.h>
#include <printf.h>
#include "tmutex.h"
#include "bthread_private.h"

int bthread_mutex_init(bthread_mutex_t *m, const bthread_mutexattr_t *attr) {
    assert(m != NULL);
    m->owner = NULL;
    m->waiting_list = NULL;
    return 0;
}

int bthread_mutex_destroy(bthread_mutex_t *m) {
    assert(m->owner == NULL);
    assert(tqueue_size(m->waiting_list) == 0);
    return 0;
}

int bthread_mutex_lock(bthread_mutex_t *m) {
    bthread_block_timer_signal();
    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    volatile __bthread_private *my_thread = (__bthread_private *) tqueue_get_data(my_scheduler->current_item);
    if (m->owner == NULL) {
        bthread_printf("(MUTACQUIRE) %d %#x\n", (int) (my_thread->arg + 1), (unsigned int) &m);
        m->owner = (void *) my_thread;
        bthread_unblock_timer_signal();
    } else {
        bthread_printf("(MUTBLOCK) %d %#x\n", (int) (my_thread->arg + 1), (unsigned int) &m);
        my_thread->state = __BTHREAD_BLOCKED;
        tqueue_enqueue(&m->waiting_list, (void *) my_thread);
        while (my_thread->state != __BTHREAD_READY)
            bthread_yield();
    }
    return 0;
}

int bthread_mutex_trylock(bthread_mutex_t *m) {
    bthread_block_timer_signal();
    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    volatile __bthread_private *my_thread = (__bthread_private *) tqueue_get_data(my_scheduler->current_item);
    if (m->owner == NULL) {
        bthread_printf("(MUTACQUIRE) %d %#x\n", (int) (my_thread->arg + 1), (unsigned int) &m);
        m->owner = (void *) my_thread;
        bthread_unblock_timer_signal();
    } else {
        bthread_unblock_timer_signal();
        return -1;
    }
    return 0;
}

int bthread_mutex_unlock(bthread_mutex_t *m) {
    bthread_block_timer_signal();
    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    assert(m->owner != NULL);
    assert(m->owner == tqueue_get_data(my_scheduler->current_item));
    volatile __bthread_private *my_thread = m->owner;
    if (my_thread != NULL) {
        bthread_printf("(MUTRELEASE) %d %#x\n", (int) (my_thread->arg + 1), (unsigned int) &m);
        my_thread->state = __BTHREAD_READY;
        if (tqueue_size(m->waiting_list) > 0) {
            m->owner = tqueue_pop((TQueue *) m->waiting_list);
        } else {
            m->owner = NULL;
        }
        bthread_yield();
        return 0;
    } else {
        m->owner = NULL;
    }
    bthread_unblock_timer_signal();
    return 0;
}
