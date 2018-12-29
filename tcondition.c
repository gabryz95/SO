#include "tcondition.h"

int bthread_cond_init(bthread_cond_t *c, const bthread_condattr_t *attr) {
    if (c == NULL) {
        c = (bthread_cond_t *) malloc(sizeof(bthread_cond_t));
        c->waiting_lits = NULL;
        return 0;
    }
    return 1;
}

int bthread_cond_destroy(bthread_cond_t *c) {
    assert(tqueue_size(c->waiting_lits) == 0);
    free(c->waiting_lits);
    return 0;
}

int bthread_cond_wait(bthread_cond_t *c, bthread_mutex_t *mutex) {
    bthread_block_timer_signal();

    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    volatile __bthread_private *my_thread = tqueue_get_data(my_scheduler->current_item);

    if (mutex->owner == my_thread) {
        volatile __bthread_private *tmp_thread = tqueue_pop(&mutex->waiting_list);
        if (tmp_thread == NULL) {
            mutex->owner = NULL;
        } else {
            tmp_thread->state = __BTHREAD_READY;
            mutex->owner = (void *) tmp_thread;
        }
    }

    my_thread->state = __BTHREAD_BLOCKED;
    tqueue_enqueue(&c->waiting_lits, (void *) my_thread);

    while (my_thread->state == __BTHREAD_BLOCKED)
        bthread_yield();

    bthread_mutex_lock(mutex);
    bthread_unblock_timer_signal();
    return 0;
}

//TODO: guardare cosa signifiva con il mutex
int bthread_cond_signal(bthread_cond_t *c) {
    bthread_block_timer_signal();
    if (tqueue_size(c->waiting_lits) > 0) {
        volatile __bthread_private *my_thread = tqueue_get_data(c->waiting_lits);
        my_thread->state = __BTHREAD_READY;
    }
    bthread_unblock_timer_signal();
    return 0;
}

int bthread_cond_broadcast(bthread_cond_t *c) {
    bthread_block_timer_signal();
    int size = (int) tqueue_size(c->waiting_lits);
    if (size > 0) {
        volatile __bthread_private *tmp_thread;
        for (int i = 0; i < size; ++i) {
            tmp_thread = (__bthread_private *) tqueue_at_offset(c->waiting_lits, (unsigned long) i);
            tmp_thread->state = __BTHREAD_READY;
        }
    }
    bthread_unblock_timer_signal();
    return 0;
}
