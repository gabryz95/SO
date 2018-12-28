#include "tbarrier.h"

int bthread_barrier_init(bthread_barrier_t *b, const bthread_barrierattr_t *attr, unsigned count) {
    assert(b != NULL);
    b->count = 0;
    b->barrier_size = count;
    b->waiting_list = NULL;
    return 0;
}

int bthread_barrier_destroy(bthread_barrier_t *b) {
    assert(tqueue_size(b->waiting_list) == 0);
    free(b->waiting_list);
    return 0;
}

int bthread_wait(bthread_barrier_t *b) {
    bthread_block_timer_signal();
    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    volatile __bthread_private *my_thread = (__bthread_private *) tqueue_get_data(my_scheduler->current_item);
    if (b->count < b->barrier_size) {
        b->count++;
        my_thread->state = __BTHREAD_BLOCKED;
        tqueue_enqueue(&b->waiting_list, (void *) my_thread);
        bthread_yield();
        printf("Aggiunto elemento alla coda della bariera\n");
    } else {
        int size = (int) tqueue_size(b->waiting_list);
        for (int i = 0; i < size; ++i) {
            __bthread_private *tmp_thread = tqueue_pop((TQueue *) b->waiting_list);
            tmp_thread->state = __BTHREAD_READY;
            printf("Tolto elemento dalla coda della bariera\n");
        }
        b->count = 0;
    }
    bthread_unblock_timer_signal();
    return 0;
}
