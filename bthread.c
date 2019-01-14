#include <setjmp.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/time.h>
#include <signal.h>
#include <printf.h>
#include "bthread.h"
#include "bthread_private.h"
#include "tqueue.h"

size_t STACK_SIZE = 10000;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////INIZIO METODI BTHREAD/////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int bthread_create(bthread_t *bthread, const bthread_attr_t *attr, void *(*start_routine)(void *), void *arg) {
    __bthread_private *bthread_private = malloc(sizeof(__bthread_private));
    bthread_private->body = start_routine;
    bthread_private->arg = arg;
    bthread_private->state = __BTHREAD_READY;
    bthread_private->stack = NULL;
    bthread_set_priority(bthread_private, 0);
    bthread_private->cancel_req = 0;

    if (attr != NULL)
        bthread_private->attr = *attr;

    volatile __bthread_scheduler_private *my_scheduler = bthread_get_scheduler();
    bthread_private->tid = tqueue_enqueue(&my_scheduler->queue, bthread_private);
    *bthread = bthread_private->tid;
    my_scheduler->current_item = my_scheduler->queue;
    bthread_printf("(CREATE) %d\n", (int) (arg + 1));
    return (int) bthread_private->tid;
}

double get_current_time_millis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    __darwin_time_t value = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
    return value;
}

int bthread_join(bthread_t bthread, void **retval) {
    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    scheduler->current_item = scheduler->queue;
    save_context(scheduler->context);
    if (bthread_check_if_zombie(bthread, retval)) return 0;
    __bthread_private *tp;
    do {
        //scheduler->current_item = tqueue_at_offset(scheduler->current_item, 1);
        scheduler->scheduling_routine();
        tp = (__bthread_private *) tqueue_get_data(scheduler->current_item);
    } while (tp->state != __BTHREAD_READY);
    bthread_printf("(SCHEDULING) %d\n", (tp->tid + 1));
    if (tp->stack) {
        restore_context(tp->context);
    } else {
        tp->stack = (char *) malloc(sizeof(char) * STACK_SIZE);
        unsigned long target = tp->stack + STACK_SIZE - 1;
#if __APPLE__
// OSX requires 16 bytes aligned stack
        target -= (target % 16);
#endif
#if __x86_64__
        asm __volatile__("movq %0, %%rsp"::"r"((intptr_t) target));
#else

        asm __volatile__("movl %0, %%esp"::"r"((intptr_t) target));
#endif
        bthread_exit(tp->body(tp->arg));

    }
}

void bthread_yield() {
    bthread_block_timer_signal();
    //get the scheduler
    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    volatile __bthread_private *thread = tqueue_get_data(scheduler->current_item);
    bthread_printf("(YIELD) %d\n", (int) (thread->arg + 1));
    if (!save_context(thread->context)) {
        bthread_printf("(INSCHEDULER)\n(RESCHEDULING)\n");
        restore_context(scheduler->context);
    }
    bthread_unblock_timer_signal();
}


void bthread_exit(void *retval) {
    volatile __bthread_scheduler_private *myScheduler = bthread_get_scheduler();
    volatile __bthread_private *my_thread = tqueue_get_data(myScheduler->current_item);
    my_thread->retval = retval;
    my_thread->state = __BTHREAD_ZOMBIE;
    bthread_printf("(ExIT) %d 0x%x\n", (int) (my_thread->arg + 1), (unsigned int) retval);
    bthread_yield();
}


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
    bthread_printf("(SLEEPING) %d\n", (int) (thread->arg + 1));
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
            my_scheduler->scheduling_routine = (bthread_scheduling_routine) random_scheduling;
            break;
        case __PRIORITY_SCHEDULER:
            my_scheduler->scheduling_routine = (bthread_scheduling_routine) priority_scheduling;
            break;
        case __ROUNDROBIN_SCHEDULER:
            my_scheduler->scheduling_routine = (bthread_scheduling_routine) round_robin_scheduler;
            break;
        case __LOTTERY_SCHEDULER:
            my_scheduler->scheduling_routine = (bthread_scheduling_routine) lottery_scheduler;
            break;
        default:
            my_scheduler->scheduling_routine = (bthread_scheduling_routine) round_robin_scheduler;
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


__bthread_scheduler_private *bthread_get_scheduler() {
    static __bthread_scheduler_private *my_scheduler = NULL;
    if (my_scheduler == NULL) {
        my_scheduler = malloc(sizeof(__bthread_scheduler_private));
        my_scheduler->queue = NULL;
        my_scheduler->scheduling_routine = (bthread_scheduling_routine) round_robin_scheduler;
    }

    return my_scheduler;
}


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

//TODO: deve ritornare il thread corrispondente, per eliminarlo.
int bthread_check_if_zombie(bthread_t bthread, void **retval) {
    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    __bthread_private *thread = tqueue_get_data(scheduler->current_item);
    if (thread->state == __BTHREAD_ZOMBIE) {
        bthread_exit(retval);
        if (retval != NULL) {
            *retval = thread->retval;
            tqueue_pop((TQueue *) thread);
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


void bthread_block_timer_signal() {
    sigset_t signal;
    sigemptyset(&signal);
    sigaddset(&signal, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &signal, NULL);

}

void bthread_unblock_timer_signal() {
    sigset_t signal;
    sigemptyset(&signal);
    sigaddset(&signal, SIGVTALRM);
    sigprocmask(SIG_UNBLOCK, &signal, NULL);
}

void bthread_set_priority(__bthread_private *bthread_private, int priority) {
    bthread_private->priority = priority;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////FINE METODI BTHREAD PRIVATE///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

