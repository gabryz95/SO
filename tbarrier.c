#include <stdlib.h>
#include <assert.h>
#include <printf.h>
#include "tbarrier.h"
#include "bthread_private.h"

int bthread_barrier_init(bthread_barrier_t *b, const bthread_barrierattr_t *attr, unsigned count) {
    assert(b != NULL);
    b->count = 0;
    b->barrier_size = count;
    b->waiting_list = NULL;
    return 0;
}

int bthread_barrier_destroy(bthread_barrier_t *b) {
    assert(tqueue_size(b->waiting_list) == 0);
    return 0;
}

int bthread_barrier_wait(bthread_barrier_t *b) {
    bthread_block_timer_signal();
    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    volatile __bthread_private *my_thread = (__bthread_private *) tqueue_get_data(my_scheduler->current_item);
    printf("(BARRIERWAIT) %d %#x %d %d\n", (int) (my_thread->arg + 1), &b, b->count + 1, b->barrier_size);
    if (b->count != b->barrier_size - 1) {
        b->count++;
        my_thread->state = __BTHREAD_BLOCKED;
        tqueue_enqueue(&b->waiting_list, (void *) my_thread);
        printf("(BARRIERBLOCKED) %d %#x %d %d\n", (int) (my_thread->arg + 1), &b, b->count, b->barrier_size);
        bthread_yield();
    } else {
        b->count++;
        printf("(BARRIERUNBLOCKED) %d %#x %d %d\n", (int) (my_thread->arg + 1), &b, b->count, b->barrier_size);
        int size = (int) tqueue_size(b->waiting_list);
        for (int i = 0; i < size; ++i) {
            __bthread_private *tmp_thread2 = tqueue_pop(&b->waiting_list);
            tmp_thread2->state = __BTHREAD_READY;
            printf("(READY) %d\n", (int) (tmp_thread2->arg + 1));
        }
        b->count = 0;
    }
    bthread_unblock_timer_signal();
    return 0;
}
