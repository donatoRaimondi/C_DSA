#ifndef QUEUE_H
#define QUEUE_H

#include "../list/list.h"
#include <err.h>
#include <stdlib.h>

typedef struct {
  list data;
  pos tail;
} queue;

/* Create an empty queue. O(1). */
queue queue_init(void);

/* Free all queue nodes; stored elem values are not freed. O(n). */
void queue_clear(queue *q);

/* Return whether the queue contains no elements. O(1). */
bool isEmpty_queue(const queue *q);

/* Add value at the back of the queue. O(1). */
void queue_push(queue *q, elem value);

/* Remove and return the front value. The queue must be nonempty. O(1). */
elem queue_pop(queue *q);

/* Return the front value without removing it. The queue must be nonempty. O(1). */
elem queue_top(const queue *q);

#endif // !QUEUE_H
