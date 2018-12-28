#include <stdlib.h>
#include "scheduler.h"
#include "bthread.h"
#include "bthread_private.h"

//scelgo a caso un elemento dalla coda che debba essere eseguito
void *random_scheduling() {
    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    volatile __bthread_private *my_thread;
    TQueue tQueue;

    do {
        unsigned long random_value = rand() % tqueue_size(my_scheduler->queue);
        tQueue = tqueue_at_offset(my_scheduler->queue, random_value);
        my_thread = tqueue_get_data(tQueue);
    } while (!my_thread->state == __BTHREAD_READY);

    my_scheduler->current_item = tQueue;
}

//chi ha la priorità maggiore sarà eseguito
void *priority_scheduling() {
    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    unsigned long size = tqueue_size(my_scheduler->queue);
    volatile unsigned int priority = 10;
    volatile __bthread_private *my_thread;
    TQueue tQueue = NULL;

    for (int i = 0; i < size; ++i) {
        TQueue tmp = tqueue_at_offset(my_scheduler->queue, (unsigned long) i);
        my_thread = tqueue_get_data(tmp);
        if (my_thread->state != __BTHREAD_EXITED)
            if (my_thread->priority < priority) {
                priority = (volatile unsigned int) my_thread->priority;
                tQueue = tmp;
            }
    }
    my_scheduler->current_item = tQueue;
}

void *round_robin_scheduler() {
    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    my_scheduler->current_item = tqueue_at_offset(my_scheduler->current_item, 1);
}


