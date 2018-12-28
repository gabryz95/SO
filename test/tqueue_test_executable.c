#include "../tqueue_test.h"

int mainc() {

    test_tqueue_assert();

    printf("\n");

    TQueue threadQueue = NULL;
    tqueue_enqueue(&threadQueue, (void *) 1);
    tqueue_enqueue(&threadQueue, (void *) 2);
    tqueue_enqueue(&threadQueue, (void *) 3);
    tqueue_enqueue(&threadQueue, (void *) 4);
    tqueue_enqueue(&threadQueue, (void *) 5);
    tqueue_enqueue(&threadQueue, (void *) 6);

    printf("Size:: %lu\n", tqueue_size(threadQueue));

    printf("Pop:: %lu\n", (unsigned long) tqueue_pop(&threadQueue));
    printf("Size:: %lu\n", tqueue_size(threadQueue));

    TQueue b = tqueue_at_offset((TQueue) &threadQueue, 3);
    printf("Data at position 3:: %lu\n", (unsigned long) b->data);

    TQueue c = tqueue_at_offset((TQueue) &threadQueue, 8);
    printf("Data at position 8:: %lu\n", (unsigned long) c->data);

    TQueue d = tqueue_at_offset((TQueue) &threadQueue, 0);
    printf("Data at position 0:: %lu\n", (unsigned long) d->data);

    return 1;
}

