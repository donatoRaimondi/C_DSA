#include "queue.h"

queue queue_init(void) {
  queue q = {.data = list_init(), .tail = NULL};

  return q;
}

void queue_clear(queue *q) {
  list_clear(&q->data);
  q->tail = NULL;
}

bool isEmpty_queue(const queue *q) { return isEmpty_list(&q->data); }

void queue_push(queue *q, elem value) {
  node *new = malloc(sizeof(node));

  if (new == NULL)
    err(EXIT_FAILURE, "queue_push: malloc failed");

  new->value = value;
  new->next = NULL;

  if (isEmpty_queue(q)) {
    q->data.head = new;
    q->tail = new;
  } else {
    q->tail->next = new;
    q->tail = new;
  }

  q->data.size++;
}

elem queue_pop(queue *q) {
  if (isEmpty_queue(q))
    errx(EXIT_FAILURE, "queue_pop: queue is empty, no element to pop");

  elem value = list_read(&q->data, list_first(&q->data));
  list_remove(&q->data, list_first(&q->data));
  if (isEmpty_queue(q))
    q->tail = NULL;

  return value;
}

elem queue_top(const queue *q) {
  if (isEmpty_queue(q))
    errx(EXIT_FAILURE, "queue_top: queue is empty, no element to read");

  return list_read(&q->data, list_first(&q->data));
}
