#include "tqueue.h"
#include <stdlib.h>
#include <stdio.h>

/* Adds a new element at the end of the list, returns its position */
unsigned long int tqueue_enqueue(TQueue *q, void *localData) {
    unsigned long position = 0;
    TQueueNode *newf = (TQueueNode *) malloc(sizeof(TQueueNode));
    newf->data = localData;
    if (*q == NULL) {
        *q = newf;
        newf->next = newf;
        return position;
    } else {
        TQueue head = *q;
        TQueue tmp = *q;
        while (tmp->next != NULL && tmp->next != head) {
            tmp = tmp->next;
            position++;
        }
        tmp->next = newf;
        newf->next = head;
        return ++position;
    }
}

/* Removes and returns the element at the beginning of the list, NULL if the queue is empty */
void *tqueue_pop(TQueue *q) {
//    if (q == NULL)
//        return NULL;
//
//    TQueue element = *q;
//    void *value = element->data;
//    TQueue head = *q;
//    TQueue tmp = *q;
//
//    if (tmp->next == head) {
//        TQueue *toRemove = (TQueue *) head;
//        *q = NULL;
//        free(toRemove);
//        return value;
//
//    }
//    while (tmp->next != head) {
//        tmp = tmp->next;
//    }
//    *q = head->next;
//    tmp->next = *q;
//    free(head);
//    return value;
    if (*q == NULL) {
        return NULL;
    } else {
        TQueueNode *old_head = *q;
        void *ret_val = old_head->data;
        if (old_head->next == old_head) {
            *q = NULL;
        } else {
            *q = old_head->next;
            TQueueNode *head = *q;
            while (head->next != old_head) {
                head = head->next;
            }
            head->next = *q;
        }
        free(old_head);
        return ret_val;
    }
}

/* Returns the number of elements in the list */
unsigned long int tqueue_size(TQueue q) {
    if (q == NULL)
        return 0;
    unsigned long cnt = 1;
    TQueueNode *head = q;
    TQueueNode *tmp = q;
    while (tmp->next != head) {
        cnt++;
        tmp = tmp->next;
    }
    return cnt;
}

/* Returns a 'view' on the list starting at (a positive) offset distance,
* NULL if the queue is empty */
TQueue tqueue_at_offset(TQueue q, unsigned long int offset) {
    if (q == NULL) {
        return NULL;
    }

    if (offset < 0)
        return NULL;

    unsigned long position;
    TQueueNode *head = q;
    for (position = 0; position < offset; position++) {
        head = head->next;
    }
    return head;
}

/* Returns the data on the first node of the given list */
void *tqueue_get_data(TQueue q) {
    if (q == NULL)
        return NULL;
    return q->data;
}