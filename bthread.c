#include <setjmp.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/time.h>
#include <signal.h>
#include "bthread.h"
#include "bthread_private.h"
#include "tqueue.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////INIZIO METODI BTHREAD/////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*Creates a new thread structure and puts it at the end of the queue. The thread identifier (stored
in the buffer pointed by bthread) corresponds to the position in the queue. The thread is not started when calling this function.
Attributes passed through the attr argument are ignored (thus it is possible to pass a NULL pointer).
Newly created threads are in the __BTHREAD_UNINITIALIZED state*/
int bthread_create(bthread_t *bthread, const bthread_attr_t *attr, void *(*start_routine)(void *), void *arg,
                   unsigned int priority) {
    __bthread_private *bthread_private = malloc(sizeof(__bthread_private));
    bthread_private->body = start_routine;
    bthread_private->arg = arg;
    bthread_private->state = __BTHREAD_UNINITIALIZED;
    bthread_private->cancel_req = 0;
    bthread_private->priority = priority;

    if (attr != NULL)
        bthread_private->attr = *attr;

    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    bthread_private->tid = tqueue_enqueue(&my_scheduler->queue, bthread_private);
    *bthread = bthread_private->tid;
    my_scheduler->current_item = my_scheduler->queue;
    return (int) bthread_private->tid;
}

/*Waits for the thread specified by bthread to terminate (i.e. __BTHREAD_ZOMBIE state), by
scheduling all the threads. In the following we will discuss some details about this procedure*/
//tempo di sveglia viene calcolato dall'ora corrente, ottenta utilizzando la seguente funzione
double get_current_time_millis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
}

//int bthread_join(bthread_t bthread, void **retval) {
//
//    bthread_block_timer_signal();
//    bthread_setup_timer();
//
//    volatile __bthread_scheduler_private *myScheduler = bthread_get_scheduler();
//    myScheduler->current_item = myScheduler->queue;
//    save_context(myScheduler->context);
//    if (bthread_check_if_zombie(bthread, retval)) return 0;
//    __bthread_private *tp;
//    do {
//        myScheduler->current_item = tqueue_at_offset(myScheduler->current_item, 1);
//        tp = (__bthread_private *) tqueue_get_data(myScheduler->current_item);
//    } while (tp->state != __BTHREAD_READY);
//    if (tp->stack) {
//        restore_context(tp->context);
//    } else {
//        tp->stack = (char *) malloc(sizeof(char) * STACK_SIZE);
//        unsigned long target = (unsigned long) (tp->stack + STACK_SIZE - 1);
//#if __APPLE__
//        // OSX requires 16 bytes aligned stack
//        target -= (target % 16);
//#endif
//#if __x86_64__
//        asm __volatile__("movq %0, %%rsp"::"r"((intptr_t) target));
//#else
//        asm __volatile__("movl %0, %%esp"::"r"((intptr_t) target));
//#endif
//        bthread_exit(tp->body(tp->arg));
//    }
//}
int bthread_join(bthread_t bthread, void **retval) {
    bthread_block_timer_signal();
    bthread_setup_timer();
    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    if (save_context(scheduler->context) == 0) {
        bthread_initialize_next();
        restore_context(scheduler->context);
    } else {
        __bthread_private *tp;
        do {
            if (bthread_check_if_zombie(bthread, retval)) return 0;

            scheduler->scheduling_routine();


            tp = (__bthread_private *) tqueue_get_data(scheduler->current_item);
            volatile __bthread_private *thread = tqueue_get_data(scheduler->current_item);
            //thread sleep implementation
            if (thread->state == __BTHREAD_SLEEPING) {
                if (thread->wake_up_time <= get_current_time_millis()) {
                    thread->state = __BTHREAD_READY;
                }
            }
        } while (tp->state != __BTHREAD_READY);
        restore_context(tp->context);
    }
}

/*
Saves the thread context and then checks whether the thread that follows in the queue is in the
__BTHREAD_UNINITIALIZED state: if so, a cushion frame is created and the corresponding thread routine is called,
otherwise bthread_yield restores (long-jumps to) the scheduler context.
Saving the thread context is achieved using sigsetjmp, which is similar to setjmp but
can also save the signal mask if the provided additional parameter is not zero (
to restore both the context and the signal mask the corresponding call is siglongjmp). Saving and restoring
the signal mask is required for implementing preemption*/
//TODO: chiede se bisogna farlo sul thread attuale o sul get the next current thread in the queue
void bthread_yield() {

    bthread_block_timer_signal();
    //get the scheduler
    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    volatile __bthread_private *thread = tqueue_get_data(scheduler->current_item);

    if (!save_context(thread->context)) {
        bthread_initialize_next();
        restore_context(scheduler->context);
    }
    bthread_unblock_timer_signal();
}

/*Terminates the calling thread and returns a value via retval that will be available to another
thread in the same process that calls bthread_join, then yields to the scheduler.
Between bthread_exit and the corresponding bthread_join the thread stays in the __BTHREAD_ZOMBIE state*/
void bthread_exit(void *retval) {
    volatile __bthread_scheduler_private *myScheduler = bthread_get_scheduler();
    volatile __bthread_private *actualThread = tqueue_get_data(myScheduler->current_item);
    actualThread->retval = retval;
    actualThread->state = __BTHREAD_ZOMBIE;
    bthread_yield();
}

//ENG
/*When calling bthread_sleep the state of the thread is set to __BTHREAD_SLEEPING and then the thread must yield to the
scheduler. The wake up time is computed from the current time, obtained using*/
//ITA
/*Quando si chiama bthread_sleep lo stato del thread è impostato su __BTHREAD_SLEEPING e quindi il thread deve restituire
allo scheduler. Il tempo di sveglia è calcolato dall'ora corrente, ottenuta usando*/
void bthread_sleep(double ms) {
    //prendo le scheduler corrente
    volatile __bthread_scheduler_private *myScheduler = bthread_get_scheduler();
    //dallo scheduler ottengo il thread che chiama sleep
    volatile __bthread_private *thread = tqueue_get_data(myScheduler->current_item);
    //imposto lo stato del thread a sleep
    thread->state = __BTHREAD_SLEEPING;
    //imposto il suo tempo di risveglio a ms + get_cuurent_time()
    thread->wake_up_time = get_current_time_millis() + ms;
    //rilascio la CPU
    bthread_yield();
}

void bthread_test_cancel(void) {
    volatile __bthread_scheduler_private *myScheduler = bthread_get_scheduler();
    volatile __bthread_private *thread = tqueue_get_data(myScheduler->current_item);
    if (thread->cancel_req == 1) {
        bthread_exit((void *) -1);
    }
}


int bthread_cancel(bthread_t bthread) {
    volatile __bthread_scheduler_private *myScheduler = bthread_get_scheduler();
    volatile TQueue tQueueElement = tqueue_at_offset(myScheduler->queue, bthread);
    volatile __bthread_private *thread_to_terminate = tqueue_get_data(tQueueElement);
    thread_to_terminate->cancel_req = 1;
}

void bthread_printf(const char *format, ...) {
    bthread_block_timer_signal();
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    bthread_unblock_timer_signal();
}

void set_scheduler(scheduler_type schedulerType) {
    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();

    switch (schedulerType) {
        case __RANDOM_SCHEDULER:
            my_scheduler->scheduling_routine = random_scheduling();
            break;
        case __PRIORITY_SCHEDULER:
            my_scheduler->scheduling_routine = priority_scheduling();
            break;
        case __ROUNDROBIN_SCHEDULER:
            my_scheduler->scheduling_routine = round_robin_scheduler();
        default:
            my_scheduler->scheduling_routine = round_robin_scheduler();
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////FINE METODI BTHREAD///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////INIZIO METODI BTHREAD PRIVATE/////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*This private function creates, maintains and returns a static pointer to the singleton instance of
__bthread_scheduler_private. Other functions will call this method to obtain this pointer.
This function should not be accessible outside the library*/
__bthread_scheduler_private *bthread_get_scheduler() {
    static __bthread_scheduler_private *my_scheduler = NULL;
    if (my_scheduler == NULL) {
        my_scheduler = malloc(sizeof(__bthread_scheduler_private));
        my_scheduler->queue = NULL;
        my_scheduler->scheduling_routine = (bthread_scheduling_routine) round_robin_scheduler;
    }
    return my_scheduler;
}

/*Frees memory allocated for threads and the scheduler. This procedure can either be called
explicitly by the programmer (before returning from the main procedure)
or automatically (as explained in the following section). This function should not be accessible outside the library*/
void bthread_cleanup() {
    __bthread_scheduler_private *myScheduler = bthread_get_scheduler();

    if (myScheduler != NULL) {
        free(myScheduler);

        __bthread_private *head = (__bthread_private *) myScheduler->current_item;
        __bthread_private *node = (__bthread_private *) myScheduler->current_item;
        __bthread_private *tmp;

        if (head != NULL) {
            while (node != head) {
                tmp = node;
                node = (__bthread_private *) myScheduler->current_item->next;
                free(tmp);
            }
        }
    }
}

/*Creates a stack cushion for the given thread, sets its state to __BTHREAD_READY, and then calls
the start routine with the argument provided through bthread_create.
The call to the start routine should be wrapped into bthread_exit to correctly handle termination*/
void bthread_create_cushion(__bthread_private *t_data) {
    unsigned char data_cushion[CUSHION_SIZE];
    data_cushion[CUSHION_SIZE - 1] = data_cushion[0];
    t_data->state = __BTHREAD_READY;
    bthread_exit(t_data->body(t_data->arg));
}

void bthread_initialize_next() {
    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    volatile TQueue tQueue_next_element = tqueue_at_offset(my_scheduler->current_item, 1);
    volatile __bthread_private *my_next_thtread = tqueue_get_data(tQueue_next_element);

    if (my_next_thtread->state == __BTHREAD_UNINITIALIZED) {
        my_scheduler->current_item = tQueue_next_element;
        bthread_create_cushion((__bthread_private *) my_next_thtread);
    }
}

//TODO: deve ritornare il thread corrispondente, per eliminarlo.
int bthread_check_if_zombie(bthread_t bthread, void **retval) {
    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    __bthread_private *thread = tqueue_get_data(scheduler->current_item);
    if (thread->state == __BTHREAD_ZOMBIE) {
        thread->state = __BTHREAD_EXITED;
        if (retval != NULL) {
            *retval = thread->retval;
        }
        return 1;
    } else {
        return 0;
    }
}

void bthread_setup_timer() {
    static bool initialized = false;

    if (!initialized) {
        signal(SIGVTALRM, (void (*)()) bthread_yield);
        struct itimerval time;
        time.it_interval.tv_sec = 0;
        time.it_interval.tv_usec = QUANTUM_USEC;
        time.it_value.tv_sec = 0;
        time.it_value.tv_usec = QUANTUM_USEC;
        initialized = true;
        setitimer(ITIMER_VIRTUAL, &time, NULL);
    }
}

/*function to avoid the problem of race condition to define two private procedures to temporarily block
the timer signal(using sigprocmask).
*/
//ENG:
/*Timer signals must be enabled when thread execution is started/resumed but must be blocked prior to any context save (sigsetjmp),
for example during bthread__yield. These procedures can be called from threads (which could be dangerous) but can also provide some
initial low-level atomicity (useful for implementing synchronization primitives). Finally, to ensure that access to the console
does not end up in a deadlock (since we are using signals), an async-safe printf function must be implemented (replacing the previous
definition of the same-name macro). Here instead of a variadic macro we use a variadic function:
 */
//ITA:
/* Timer signals must be enabled when thread execution is started/resumed but must be blocked prior to any context save (sigsetjmp),
for example during bthread__yield. These procedures can be called from threads (which could be dangerous) but can also provide some
initial low-level atomicity (useful for implementing synchronization primitives). Finally, to ensure that access to the console does
not end up in a deadlock (since we are using signals), an async-safe printf function must be implemented (replacing the previous
definition of the same-name macro). Here instead of a variadic macro we use a variadic function:
 */
void bthread_block_timer_signal() {
    sigset_t signal;
    sigemptyset(&signal);
    sigaddset(&signal, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &signal, NULL);

}

/*function to avoid the problem of race condition to define two private procedures to temporarily unblock
the timer signal(using sigprocmask).
*/
void bthread_unblock_timer_signal() {
    sigset_t signal;
    sigemptyset(&signal);
    sigaddset(&signal, SIGVTALRM);
    sigprocmask(SIG_UNBLOCK, &signal, NULL);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////FINE METODI BTHREAD PRIVATE///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

