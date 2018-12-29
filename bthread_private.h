#ifndef OPERATIVE_SISTEM_BTHREAD_PRIVATE_H
#define OPERATIVE_SISTEM_BTHREAD_PRIVATE_H

#include <setjmp.h>
#include <stdbool.h>
#include "tqueue.h"

#define save_context(CONTEXT) sigsetjmp(CONTEXT, 1)
#define restore_context(CONTEXT) siglongjmp(CONTEXT, 1)
#define QUANTUM_USEC 20000
#define CUSHION_SIZE 10000

/* This structure specifies the signature of the thread body routine. */
typedef void *(*bthread_routine)(void *);

/* Type for thread context */
typedef unsigned long int bthread_t;

/* Container for thread context. This structure can't change, otherwise there could be conflicts. */
typedef enum {
    __BTHREAD_EXITED = 0,
    __BTHREAD_ZOMBIE,
    __BTHREAD_UNINITIALIZED,
    __BTHREAD_READY,
    __BTHREAD_BLOCKED,
    __BTHREAD_SLEEPING
} bthread_state;


/* This structure store additional thread attribute. */
typedef struct {
} bthread_attr_t;

/* This structure contains all the information regarding a thread (an identifier, the body routine and its argument,
 * an execution state, attributes, stack context, and a return value). */
typedef void (*bthread_scheduling_routine)();

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

static void bthread_create_cushion(__bthread_private *t_data);

static void bthread_initialize_next();

static int bthread_check_if_zombie(bthread_t bthread, void **retval);

static void bthread_setup_timer();

void bthread_block_timer_signal();

void bthread_unblock_timer_signal();

#endif //OPERATIVE_SISTEM_BTHREAD_PRIVATE_H
