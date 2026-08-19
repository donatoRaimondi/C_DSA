#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static unsigned tests_run;
static unsigned tests_failed;

#define CHECK(condition)                                                       \
  do {                                                                         \
    ++tests_run;                                                               \
    if (!(condition)) {                                                        \
      ++tests_failed;                                                          \
      fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);     \
    }                                                                          \
  } while (0)

typedef void (*death_test_fn)(void);

static void check_exits_with_failure(death_test_fn fn, const char *name) {
  pid_t child = fork();
  if (child == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (child == 0) {
    (void)freopen("/dev/null", "w", stderr);
    fn();
    _Exit(EXIT_SUCCESS);
  }

  int status = 0;
  if (waitpid(child, &status, 0) == -1) {
    perror("waitpid");
    exit(EXIT_FAILURE);
  }

  ++tests_run;
  if (!WIFEXITED(status) || WEXITSTATUS(status) == EXIT_SUCCESS) {
    ++tests_failed;
    fprintf(stderr, "FAIL: %s did not exit with failure\n", name);
  }
}

static void check_empty_invariants(const queue *q) {
  CHECK(isEmpty_queue(q));
  CHECK(q->data.size == 0);
  CHECK(q->data.head == NULL);
  CHECK(q->tail == NULL);
}

static void test_empty_queue(void) {
  queue q = queue_init();
  check_empty_invariants(&q);
}

static void test_push_and_top(void) {
  int a = 10;
  int b = 20;
  queue q = queue_init();

  queue_push(&q, &a);
  CHECK(!isEmpty_queue(&q));
  CHECK(q.data.size == 1);
  CHECK(q.data.head == q.tail);
  CHECK(q.tail->next == NULL);
  CHECK(queue_top(&q) == &a);

  pos original_head = q.data.head;
  queue_push(&q, &b);
  CHECK(q.data.size == 2);
  CHECK(q.data.head == original_head);
  CHECK(q.data.head->next == q.tail);
  CHECK(q.tail->value == &b);
  CHECK(q.tail->next == NULL);
  CHECK(queue_top(&q) == &a);
  CHECK(q.data.size == 2);

  queue_clear(&q);
}

static void test_fifo_order(void) {
  int values[] = {1, 2, 3, 4, 5};
  queue q = queue_init();

  for (size_t i = 0; i < 5; ++i) {
    queue_push(&q, &values[i]);
    CHECK(queue_top(&q) == &values[0]);
    CHECK(q.data.size == i + 1);
    CHECK(q.tail->value == &values[i]);
    CHECK(q.tail->next == NULL);
  }

  for (size_t i = 0; i < 5; ++i) {
    CHECK(queue_top(&q) == &values[i]);
    CHECK(queue_pop(&q) == &values[i]);
    CHECK(q.data.size == 4 - i);
  }

  check_empty_invariants(&q);
}

static void test_interleaved_operations(void) {
  int a = 1;
  int b = 2;
  int c = 3;
  int d = 4;
  queue q = queue_init();

  queue_push(&q, &a);
  queue_push(&q, &b);
  CHECK(queue_pop(&q) == &a);
  queue_push(&q, &c);
  queue_push(&q, &d);
  CHECK(queue_top(&q) == &b);
  CHECK(queue_pop(&q) == &b);
  CHECK(queue_pop(&q) == &c);
  CHECK(queue_pop(&q) == &d);
  check_empty_invariants(&q);
}

static void test_null_element(void) {
  int value = 7;
  queue q = queue_init();

  queue_push(&q, NULL);
  queue_push(&q, &value);
  CHECK(!isEmpty_queue(&q));
  CHECK(queue_top(&q) == NULL);
  CHECK(queue_pop(&q) == NULL);
  CHECK(queue_top(&q) == &value);
  CHECK(queue_pop(&q) == &value);
  check_empty_invariants(&q);
}

static void test_single_element_cycles(void) {
  int values[] = {1, 2, 3};
  queue q = queue_init();

  for (size_t i = 0; i < 3; ++i) {
    queue_push(&q, &values[i]);
    CHECK(q.data.head == q.tail);
    CHECK(queue_pop(&q) == &values[i]);
    check_empty_invariants(&q);
  }
}

static void test_clear_and_reuse(void) {
  int values[] = {1, 2, 3};
  queue q = queue_init();

  queue_clear(&q);
  check_empty_invariants(&q);

  for (size_t i = 0; i < 3; ++i)
    queue_push(&q, &values[i]);
  queue_clear(&q);
  check_empty_invariants(&q);

  /* Clearing nodes must not destroy caller-owned values. */
  CHECK(values[0] == 1);
  CHECK(values[1] == 2);
  CHECK(values[2] == 3);

  queue_clear(&q);
  queue_push(&q, &values[2]);
  CHECK(q.data.head == q.tail);
  CHECK(queue_pop(&q) == &values[2]);
  check_empty_invariants(&q);
}

static void dies_pop_empty(void) {
  queue q = queue_init();
  (void)queue_pop(&q);
}

static void dies_top_empty(void) {
  queue q = queue_init();
  (void)queue_top(&q);
}

static void test_invalid_operations(void) {
  check_exits_with_failure(dies_pop_empty, "queue_pop on empty queue");
  check_exits_with_failure(dies_top_empty, "queue_top on empty queue");
}

int main(void) {
  test_empty_queue();
  test_push_and_top();
  test_fifo_order();
  test_interleaved_operations();
  test_null_element();
  test_single_element_cycles();
  test_clear_and_reuse();
  test_invalid_operations();

  if (tests_failed != 0) {
    fprintf(stderr, "%u/%u tests failed\n", tests_failed, tests_run);
    return EXIT_FAILURE;
  }

  printf("All %u queue tests passed\n", tests_run);
  return EXIT_SUCCESS;
}
