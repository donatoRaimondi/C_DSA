#include "priority_queue.h"
#include <err.h>
#include <stdlib.h>

priority_queue priority_queue_init(size_t capacity, compare_fn compare) {
  if (capacity == 0)
    errx(EXIT_FAILURE, "priority_queue_init: capacity must be > 0");

  if (compare == NULL)
    errx(EXIT_FAILURE, "priority_queue_init: compare must not be NULL");

  priority_queue pq = {.data = calloc(capacity, sizeof *pq.data),
                       .size = 0,
                       .capacity = capacity,
                       .compare = compare};

  if (pq.data == NULL)
    err(EXIT_FAILURE, "priority_queue_init: allocation failed");

  return pq;
}

void priority_queue_clear(priority_queue *pq) {
  free(pq->data);

  pq->data = NULL;
  pq->size = 0;
  pq->capacity = 0;
  pq->compare = NULL;
}

bool isEmpty_priority_queue(const priority_queue *pq) { return pq->size == 0; }

static void sift_up(priority_queue *pq, size_t i) {
  while (i > 0) {
    size_t parent = (i - 1) / 2;

    if (pq->compare(pq->data[i], pq->data[parent]) >= 0)
      break;

    elem temp = pq->data[i];
    pq->data[i] = pq->data[parent];
    pq->data[parent] = temp;

    i = parent;
  }
}

void priority_queue_push(priority_queue *pq, elem value) {
  if (pq->size == pq->capacity) {
    size_t new_capacity = pq->capacity * 2;

    elem *new_data = realloc(pq->data, new_capacity * sizeof *pq->data);

    if (new_data == NULL)
      err(EXIT_FAILURE, "priority_queue_push: realloc failed");

    pq->data = new_data;
    pq->capacity = new_capacity;
  }

  size_t i = pq->size;

  pq->data[i] = value;
  pq->size++;

  sift_up(pq, i);
}

static void sift_down(priority_queue *pq, size_t i) {
  while (1) {
    size_t left = 2 * i + 1;
    size_t right = 2 * i + 2;
    size_t smallest = i;

    if (left < pq->size && pq->compare(pq->data[left], pq->data[smallest]) < 0)
      smallest = left;

    if (right < pq->size &&
        pq->compare(pq->data[right], pq->data[smallest]) < 0)
      smallest = right;

    if (smallest == i)
      break;

    elem tmp = pq->data[i];
    pq->data[i] = pq->data[smallest];
    pq->data[smallest] = tmp;

    i = smallest;
  }
}

elem priority_queue_pop(priority_queue *pq) {
  if (isEmpty_priority_queue(pq))
    return NULL;

  elem value = pq->data[0];

  pq->size--;

  if (pq->size > 0) {
    pq->data[0] = pq->data[pq->size];
    sift_down(pq, 0);
  }

  return value;
}

elem priority_queue_peek(const priority_queue *pq) {
  return isEmpty_priority_queue(pq) ? NULL : pq->data[0];
}
