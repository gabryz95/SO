#include "tqueue_test.h"
#include <assert.h>

void test_tqueue_assert() {
    TQueue tqueue_t = NULL;
    /* Test all method without enqueue a element */
    printf("Start Test file tqueue\n");

    printf("assert(tqueue_size(NULL) == 0)\n");
    assert(tqueue_size(NULL) == 0);

    printf("assert(tqueue_at_offset(NULL, 0) == NULL)\n");
    assert(tqueue_at_offset(NULL, 0) == NULL);

    printf("assert(tqueue_pop(NULL) == NULL)\n");
    assert(tqueue_pop(NULL) == NULL);

    printf("assert(tqueue_get_data(NULL) == NULL)\n");
    assert(tqueue_get_data(NULL) == NULL);

    /* enqueue a new element and test function */
    printf("add elemnt 3 to Tqueue * t\n");
    unsigned long a = 3;
    printf("tqueue_enqueue(&t, 3)==0\n");
    assert(tqueue_enqueue(&tqueue_t, &a) == 0);

    /*tested 100% coverage of method tqueue_get_data(). */
    printf("assert(tqueue_get_data(t) == 3)\n");
    assert((*(unsigned long *) tqueue_get_data(tqueue_t)) == 3);

    TQueue tmp = tqueue_at_offset(tqueue_t, 0);
    printf("tqueue_at_offset(t, 0) == t)\n");
    assert((*(unsigned long *) (tmp->data)) == (*(unsigned long *) (tqueue_t->data)));
    printf("tqueue_at_offset(t, 5) == NULL)\n");

    printf("assert(tqueue_size(t) == 1)\n");
    assert(tqueue_size(tqueue_t) == 1);

    printf("assert(tqueue_pop(t) == 3)\n");
    unsigned long b = *(unsigned long *) tqueue_pop(&tqueue_t);
    assert(b == 3);
    printf("assert(t == NULL)\n");
    assert(tqueue_t == NULL);
    printf("Well done! all test passed!\n");

    /* enqueue two new element and test function */
    printf("add elemnt 3 to Tqueue * t\n");
    unsigned long  a1 = 3;
    unsigned long a2 = 6;

    printf("tqueue_enqueue(&t, 3)==0\n");
    assert(tqueue_enqueue(&tqueue_t, &a1) == 0);

    printf("tqueue_enqueue(&t, 6)==0\n");
    assert(tqueue_enqueue(&tqueue_t, &a2) == 1);

    TQueue tmp1 = tqueue_at_offset(tqueue_t, 0);
    printf("tqueue_at_offset(t, 0) == t)\n");
    assert((*(unsigned long *) (tmp1->data)) == (*(unsigned long *) (tqueue_t->data)));

    TQueue tmp2 = tqueue_at_offset(tqueue_t, 1);
    printf("tqueue_at_offset(t, 1) == t)\n");
    assert((*(unsigned long *) (tmp2->data)) == (*(unsigned long *) (tqueue_t->next->data)));

    printf("assert(tqueue_size(t) == 2)\n");
    assert(tqueue_size(tqueue_t) == 2);

    printf("assert(tqueue_pop(t) == 3)\n");
    unsigned long b1 = *(unsigned long *) tqueue_pop(&tqueue_t);
    assert(b1 == 3);

    printf("assert(tqueue_pop(t) == 3)\n");
    unsigned long b2 = *(unsigned long *) tqueue_pop(&tqueue_t);
    assert(b2 == 6);

    printf("assert(t == NULL)\n");
    assert(tqueue_t == NULL);
    printf("Well done! all test passed!\n");
}
