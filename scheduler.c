#include <stdlib.h>
#include <printf.h>
#include <sys/time.h>
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
    volatile unsigned int priority = 0;
    volatile __bthread_private *my_thread;
    TQueue tQueue = NULL;

    for (int i = 0; i < size; ++i) {
        TQueue tmp = tqueue_at_offset(my_scheduler->queue, (unsigned long) i);
        my_thread = tqueue_get_data(tmp);
        if (my_thread->state != __BTHREAD_ZOMBIE)
            if (my_thread->priority < priority) {
                priority = (volatile unsigned int) my_thread->priority;
                tQueue = tmp;
                my_scheduler->current_item = tQueue;
            }
    }
    //TODO:: è NULL perchè non è entrato dentro if (my_thread->priority < priority)
    //my_scheduler->current_item = tQueue;
}

void *round_robin_scheduler() {
    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    TQueue my_TQueue = tqueue_at_offset(my_scheduler->current_item, 1);
    __bthread_private * my_thread = tqueue_get_data(my_TQueue);

    double value = get_current_time_millis();
    if (value > my_thread->wake_up_time && my_thread->state != __BTHREAD_READY) {
        my_thread->state = __BTHREAD_READY;
        bthread_printf("(READY) %d\n", (my_thread->arg + 1));
    }
    my_scheduler->current_item = my_TQueue;
}

//TODO: controlare se lo stato è ready e metterlo in un array locale edarlie i biglietti in base alla prio
void *lottery_scheduler() {
    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    volatile __bthread_private *my_thread;
    unsigned long size = tqueue_size(my_scheduler->queue);
    int lottery[size], ticket[size][size];
    int ticket_value = 1, m_ticket = 0;
    TQueue tQueue = NULL;

    for (int l = 0; l < size; ++l) {
        tQueue = tqueue_at_offset(my_scheduler->queue, l);
        my_thread = tqueue_get_data(tQueue);
        if (my_thread->state == __BTHREAD_READY) {
            //aggiungere alla lista
        }
    }


    //sorting burst time, priority and process numenr ina scending order using selection sort
    for (int j = 0; j < size; ++j) {
        tQueue = tqueue_at_offset(my_scheduler->queue, j);
        my_thread = tqueue_get_data(tQueue);
        lottery[j] = my_thread->priority;
        for (int i = 0; i < lottery[j]; ++i) {
            ticket[j][i] = ticket_value++;
            m_ticket = ticket_value;
        }
    }

    //select new current item for scheduler
    int winner = (rand() % m_ticket - 1) + 1;
    for (int k = 0; k < size; ++k) {
        for (int i = 0; i < size; ++i) {
            if (ticket[k][i] == winner) {
                my_scheduler->current_item = tqueue_at_offset(my_scheduler->queue, (unsigned long) k);
                break;
            }
        }
    }
}

void task_set_priority(int priority) {
    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    volatile __bthread_private *my_thread = tqueue_get_data(my_scheduler->current_item);
    my_thread->priority = priority;
}