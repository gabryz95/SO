#include <stdlib.h>
#include <printf.h>
#include "scheduler.h"
#include "bthread.h"
#include "bthread_private.h"

int flag = 0;

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

void *lottery_scheduler() {
    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    volatile __bthread_private *my_thread;
    unsigned long size = tqueue_size(my_scheduler->queue);
    int lottery[size], ticket[size][size];
    int qauntum = 1, ticket_value = 1, m_ticket = 0;
    TQueue tQueue = NULL;


    //assigne quantum time for expire process of each thread
    if (flag == 0) {
        for (int i = 0; i < size; ++i) {
            tQueue = tqueue_at_offset(my_scheduler->queue, i);
            my_thread = tqueue_get_data(tQueue);
            if (my_thread->state != __BTHREAD_EXITED) {
                my_thread->burst = (rand() % 5) + 1;
            }
        }
        flag = 1;
    }

    //sorting burst time, priority and process numenr ina scending order using selection sort
    for (int j = 0; j < size; ++j) {
        tQueue = tqueue_at_offset(my_scheduler->queue, j);
        my_thread = tqueue_get_data(tQueue);
        lottery[j] = (int) ((my_thread->burst / qauntum) + (size - my_thread->priority));
        for (int i = 0; i < lottery[j]; ++i) {
            ticket[j][i] = ticket_value++;
            m_ticket = ticket_value;
        }
    }

    //select new current item for scheduler
    int winner = (rand() % m_ticket - 1) + 1;
    for (int k = 0; k < size; ++k) {
        for (int i = 0; i < size; ++i) {
            if (ticket[k][i] == winner)
                my_scheduler->current_item = tqueue_at_offset(my_scheduler->queue, (unsigned long) k);
            my_thread = tqueue_get_data(tqueue_at_offset(my_scheduler->queue, (unsigned long) k));
            if (my_thread->burst > 0) {
                my_thread->burst -= qauntum;
            } else
                my_thread->burst = 0;
        }
    }
}

void task_set_priority(int priority) {
    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    volatile __bthread_private *my_thread = tqueue_get_data(my_scheduler->current_item);
    my_thread->priority = priority;

}


