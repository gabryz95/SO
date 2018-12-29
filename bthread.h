#ifndef OPERATIVE_SISTEM_BTHREAD_H
#define OPERATIVE_SISTEM_BTHREAD_H

#include <setjmp.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/time.h>
#include <signal.h>
#include "bthread_private.h"
#include "tqueue.h"
#include "scheduler.h"

/**
 * Implementation:
 * There is no state to represents a running thread because this information is maintained directly by the scheduler
 * */

//vecchia primitiva, adesso sostiyuita dalla funzione bthread_printf.
/*#define bthread_printf(...)\
printf(__VA_ARGS__);\
bthread_yield();*/

int bthread_create(bthread_t *bthread, const bthread_attr_t *attr, void *(*start_routine)(void *), void *arg,
                   unsigned int priority);

int bthread_join(bthread_t bthread, void **retval);

void bthread_yield();

void bthread_exit(void *retval);

void bthread_sleep(double ms);

int bthread_cancel(bthread_t bthread);

void bthread_test_cancel(void);

void bthread_printf(const char *format, ...);

void set_scheduler(scheduler_type schedulerType);

#endif //OPERATIVE_SISTEM_BTHREAD_H
