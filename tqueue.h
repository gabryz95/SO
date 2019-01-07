#ifndef OPERATIVE_SISTEM_TQUEUE_H
#define OPERATIVE_SISTEM_TQUEUE_H

/* For usage*/
typedef struct TQueueNode *TQueue;
struct TQueueNode;

/* Global TQueue*/
/* Structure of Queue Node, where data contains the reference to a long int values */
typedef struct TQueueNode {
    struct TQueueNode *next;
    void *data;
} TQueueNode;

/* Adds a new element at the end of the list, returns its position */
unsigned long int tqueue_enqueue(TQueue *q, void *data);

/* Removes and returns the element at the beginning of the list, NULL if the queue is empty */
void *tqueue_pop(TQueue *q);

/* Returns the number of elements in the list */
unsigned long int tqueue_size(TQueue q);

/* Returns a 'view' on the list starting at (a positive) offset distance,
* NULL if the queue is empty */
TQueue tqueue_at_offset(TQueue q, unsigned long int offset);

/* Returns the data on the first node of the given list */
void *tqueue_get_data(TQueue q);

#endif //OPERATIVE_SISTEM_TQUEUE_H
