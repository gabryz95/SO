#ifndef OPERATIVE_SISTEM_BTHREAD_H
#define OPERATIVE_SISTEM_BTHREAD_H

/**
 * Implementation:
 * There is no state to represents a running thread because this information is maintained directly by the scheduler
 * */

#include "scheduler.h"

/* This structure specifies the signature of the thread body routine. */
typedef void *(*bthread_routine)(void *);

/* Type for thread context */
typedef unsigned long int bthread_t;

typedef void (*bthread_scheduling_routine)();

/* This structure store additional thread attribute. */
typedef struct {
} bthread_attr_t;

int bthread_create(bthread_t *bthread, const bthread_attr_t *attr, void *(*start_routine)(void *), void *arg);

int bthread_join(bthread_t bthread, void **retval);

void bthread_yield();

void bthread_exit(void *retval);

void bthread_sleep(double ms);

int bthread_cancel(bthread_t bthread);

void bthread_test_cancel(void);

void bthread_printf(const char *format, ...);

void set_scheduler(scheduler_type schedulerType);

#endif //OPERATIVE_SISTEM_BTHREAD_H
