#include "tsemaphore.h"

int bthread_sem_init(bthread_sem_t *m, int pshared, int value) {
    if (m == NULL)
        m = (bthread_sem_t *) malloc(sizeof(bthread_sem_t));
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
    if (m->value > 0) {
        m->value--;
        bthread_unblock_timer_signal();
        return 0;
    }
    return -1;
}

int bthread_sem_post(bthread_sem_t *m) {
    bthread_block_timer_signal();
    m->value++;
    bthread_unblock_timer_signal();
    return 0;
}


