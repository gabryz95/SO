#include "tmutex.h"

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
        m->owner = my_thread;
        bthread_unblock_timer_signal();
    } else {
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
        m->owner = my_thread;
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
    volatile __bthread_private *my_thread = tqueue_pop(&m->waiting_list);
    if (my_thread != NULL) {
        m->owner = my_thread;
        my_thread->state = __BTHREAD_READY;
        bthread_yield();
        return 0;
    } else {
        m->owner = NULL;
    }
    bthread_unblock_timer_signal();
    return 0;
}
