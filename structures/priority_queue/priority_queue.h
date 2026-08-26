#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stdbool.h>
#include <stddef.h>

typedef void *elem;

typedef int (*compare_fn)(const elem left, const elem right);

typedef struct {
  elem *data;
  size_t size;
  size_t capacity;
  compare_fn compare;
} priority_queue;

/* Create an empty heap. capacity > 0 and compare != NULL are required. */
priority_queue priority_queue_init(size_t capacity, compare_fn compare);

/* Free the heap array and reset all fields; stored values are not freed. */
void priority_queue_clear(priority_queue *pq);

/* Return whether the queue contains no values. O(1). */
bool isEmpty_priority_queue(const priority_queue *pq);

/* Insert value, growing the backing array when full. O(log n) amortized. */
void priority_queue_push(priority_queue *pq, elem value);

/* Remove and return the highest-priority value, or NULL when empty. O(log n). */
elem priority_queue_pop(priority_queue *pq);

/* Return the highest-priority value, or NULL when empty. O(1). */
elem priority_queue_peek(const priority_queue *pq);

#endif // !PRIORITY_QUEUE_H
