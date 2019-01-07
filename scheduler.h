
#ifndef OPERATIVE_SISTEM_SCHEDULER_H
#define OPERATIVE_SISTEM_SCHEDULER_H

typedef enum {
    __RANDOM_SCHEDULER, __PRIORITY_SCHEDULER, __ROUNDROBIN_SCHEDULER, __LOTTERY_SCHEDULER
} scheduler_type;

void *random_scheduling();

void *priority_scheduling();

void *round_robin_scheduler();

void *lottery_scheduler();

void task_set_priority(int priority);

#endif //OPERATIVE_SISTEM_SCHEDULER_H
