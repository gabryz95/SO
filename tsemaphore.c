#include <stdlib.h>
#include <assert.h>
#include <printf.h>
#include "bthread_private.h"
#include "tsemaphore.h"

static int cnt = 0;

int bthread_sem_init(bthread_sem_t *m, int pshared, int value) {
    cnt++;
    assert(m != NULL);
    m->value = value;
    m->waiting_list = NULL;
    return 0;
}

int bthread_sem_destroy(bthread_sem_t *m) {
    assert(tqueue_size(m->waiting_list) == 0);
    free(m->waiting_list);
    return 0;
}

int bthread_sem_wait(bthread_sem_t *m) {
    bthread_block_timer_signal();
    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    volatile __bthread_private *my_thread = (__bthread_private *) tqueue_get_data(my_scheduler->current_item);
    bthread_printf("(SEMWAIT) %d %#x %d\n", (int) (my_thread->arg + 1), (unsigned int) &m, m->value);
    if (m->value > 0) {
        m->value--;
        bthread_printf("(SEMACQUIRE) %d %#x %d\n", (int) (my_thread->arg + 1), (unsigned int) &m, m->value);
    } else {
        printf("(SEMBLOCK) %d %#x %d\n", (int) (my_thread->arg + 1), (unsigned int) &m, m->value);
        tqueue_enqueue(&m->waiting_list, (void *) my_thread);
    }
    bthread_unblock_timer_signal();
    return 0;
}

int bthread_sem_post(bthread_sem_t *m) {
    bthread_block_timer_signal();
    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    volatile __bthread_private *my_thread = (__bthread_private *) tqueue_get_data(my_scheduler->current_item);
    m->value++;
    bthread_printf("(SEMPOST) %d %#x %d\n", (int) (my_thread->arg + 1), (unsigned int) &m, m->value);
    if (tqueue_size(m->waiting_list) > 0) {
        tqueue_pop(&m->waiting_list);
    }
    bthread_unblock_timer_signal();
    return 0;
}


