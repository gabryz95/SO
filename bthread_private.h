#ifndef OPERATIVE_SISTEM_BTHREAD_PRIVATE_H
#define OPERATIVE_SISTEM_BTHREAD_PRIVATE_H

#include <setjmp.h>
#include <stdbool.h>
#include "bthread.h"
#include "tqueue.h"

#define save_context(CONTEXT) sigsetjmp(CONTEXT, 1)
#define restore_context(CONTEXT) siglongjmp(CONTEXT, 1)
#define QUANTUM_USEC 20000

/* Container for thread context. This structure can't change, otherwise there could be conflicts. */
typedef enum {
    __BTHREAD_READY = 0,
    __BTHREAD_ZOMBIE,
    __BTHREAD_BLOCKED,
    __BTHREAD_SLEEPING
} bthread_state;

/* This structure contains all the information regarding a thread (an identifier, the body routine and its argument,
 * an execution state, attributes, stack context, and a return value). */
// struttura per la gestione del bthread private
typedef struct {
    bthread_t tid;
    bthread_routine body;
    void *arg;
    bthread_state state;
    bthread_attr_t attr;
    char *stack;
    jmp_buf context;
    void *retval;
    int cancel_req;     //flag per le richieste di cancellazione
    double wake_up_time;    //numero di ms che il thread deve dormire
    int priority;
    int burst;
} __bthread_private;

//struttura per la gestione di alcuni dati privati dello scheduler
typedef struct {
    TQueue queue;
    TQueue current_item;
    jmp_buf context;
    bthread_t current_tid;
    bthread_scheduling_routine scheduling_routine;
} __bthread_scheduler_private;


//metodi
__bthread_scheduler_private *bthread_get_scheduler();

void bthread_cleanup();

static int bthread_check_if_zombie(bthread_t bthread, void **retval);

static void bthread_setup_timer();

void bthread_block_timer_signal();

void bthread_unblock_timer_signal();

void bthread_set_priority(__bthread_private *bthread_private, int priority);

double get_current_time_millis();

#endif //OPERATIVE_SISTEM_BTHREAD_PRIVATE_H
