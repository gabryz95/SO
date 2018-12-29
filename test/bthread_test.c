#include <printf.h>
#include "../bthread.h"


void *testRoutine1(void *arg) {
    volatile int i;

    for (i = 0; i < 10; i++) {

        printf("BThread1, i=%d\n", i);
        bthread_sleep(2);
        bthread_printf("");
    }
}

void *testRoutine2(void *arg) {
    volatile int i;

    for (i = 0; i < 10; i++) {
        printf("BThread2, i=%d\n", i);
        bthread_sleep(2);
        bthread_printf("");
    }
}

void *testRoutine3(void *arg) {
    volatile int i;

    for (i = 0; i < 10; i++) {
        printf("BThread3, i=%d\n", i);
        bthread_sleep(2);
        bthread_printf("");
    }
}

void *testRoutine4(void *arg) {
    volatile int i;

    for (i = 0; i < 10; i++) {
        printf("BThread4, i=%d\n", i);
        bthread_sleep(2);
        bthread_printf("");
    }
}


void cmain() {
    //seleziono il tipo di scheduler da usare, OPZIONALE (se non impostato = ROUND ROBIN)
    // altre possibilità __SCHEDULER_RANDOM , __SCHEDULER_PRIORITY , __SCHEDULER_ROUNDROBIN
    set_scheduler(__ROUNDROBIN_SCHEDULER);

    unsigned long int bid1;
    unsigned long int bid2;
    unsigned long int bid3;
    unsigned long int bid4;

    bthread_create(&bid1, NULL, testRoutine1, NULL, 7);
    bthread_create(&bid2, NULL, testRoutine2, NULL, 4);
    bthread_create(&bid3, NULL, testRoutine3, NULL, 3);
    bthread_create(&bid4, NULL, testRoutine4, NULL, 1);

    bthread_join(bid1, NULL);
    bthread_join(bid2, NULL);
    bthread_join(bid3, NULL);
    bthread_join(bid4, NULL);


}
