#include "priority_queue.h"

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

static int compare_ints(const elem left, const elem right) {
  const int a = *(const int *)left;
  const int b = *(const int *)right;
  return (a > b) - (a < b);
}

static int compare_ints_descending(const elem left, const elem right) {
  return -compare_ints(left, right);
}

static int compare_nullable_ints(const elem left, const elem right) {
  if (left == right)
    return 0;
  if (left == NULL)
    return -1;
  if (right == NULL)
    return 1;
  return compare_ints(left, right);
}

static void check_heap_invariant(const priority_queue *pq) {
  CHECK(pq->size <= pq->capacity);
  for (size_t child = 1; child < pq->size; ++child) {
    size_t parent = (child - 1) / 2;
    CHECK(pq->compare(pq->data[parent], pq->data[child]) <= 0);
  }
}

static void test_initialization(void) {
  priority_queue pq = priority_queue_init(4, compare_ints);

  CHECK(pq.data != NULL);
  CHECK(pq.size == 0);
  CHECK(pq.capacity == 4);
  CHECK(pq.compare == compare_ints);
  CHECK(isEmpty_priority_queue(&pq));
  CHECK(priority_queue_peek(&pq) == NULL);
  CHECK(priority_queue_pop(&pq) == NULL);
  for (size_t i = 0; i < pq.capacity; ++i)
    CHECK(pq.data[i] == NULL);

  priority_queue_clear(&pq);
}

static void test_push_and_peek(void) {
  int values[] = {8, 3, 10, -2, 5};
  priority_queue pq = priority_queue_init(8, compare_ints);

  for (size_t i = 0; i < 5; ++i) {
    priority_queue_push(&pq, &values[i]);
    CHECK(pq.size == i + 1);
    check_heap_invariant(&pq);
  }
  CHECK(priority_queue_peek(&pq) == &values[3]);
  CHECK(pq.size == 5);

  priority_queue_clear(&pq);
}

static void test_pop_sorted_order(void) {
  int values[] = {7, -4, 7, 0, 12, 3, -1, 3};
  const int expected[] = {-4, -1, 0, 3, 3, 7, 7, 12};
  priority_queue pq = priority_queue_init(8, compare_ints);

  for (size_t i = 0; i < 8; ++i)
    priority_queue_push(&pq, &values[i]);

  for (size_t i = 0; i < 8; ++i) {
    CHECK(*(int *)priority_queue_peek(&pq) == expected[i]);
    CHECK(*(int *)priority_queue_pop(&pq) == expected[i]);
    CHECK(pq.size == 7 - i);
    check_heap_invariant(&pq);
  }
  CHECK(isEmpty_priority_queue(&pq));
  CHECK(priority_queue_pop(&pq) == NULL);

  priority_queue_clear(&pq);
}

static void test_automatic_growth(void) {
  int values[] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
  priority_queue pq = priority_queue_init(1, compare_ints);

  for (size_t i = 0; i < 9; ++i) {
    priority_queue_push(&pq, &values[i]);
    CHECK(pq.capacity >= pq.size);
    check_heap_invariant(&pq);
  }
  CHECK(pq.capacity == 16);
  for (int expected = 1; expected <= 9; ++expected)
    CHECK(*(int *)priority_queue_pop(&pq) == expected);

  priority_queue_clear(&pq);
}

static void test_custom_priority(void) {
  int values[] = {1, 5, 3, 9, 2};
  const int expected[] = {9, 5, 3, 2, 1};
  priority_queue pq = priority_queue_init(3, compare_ints_descending);

  for (size_t i = 0; i < 5; ++i)
    priority_queue_push(&pq, &values[i]);
  for (size_t i = 0; i < 5; ++i)
    CHECK(*(int *)priority_queue_pop(&pq) == expected[i]);

  priority_queue_clear(&pq);
}

static void test_interleaved_operations(void) {
  int values[] = {5, 2, 8, 1};
  priority_queue pq = priority_queue_init(2, compare_ints);

  priority_queue_push(&pq, &values[0]);
  priority_queue_push(&pq, &values[1]);
  CHECK(priority_queue_pop(&pq) == &values[1]);
  priority_queue_push(&pq, &values[2]);
  priority_queue_push(&pq, &values[3]);
  CHECK(priority_queue_pop(&pq) == &values[3]);
  CHECK(priority_queue_pop(&pq) == &values[0]);
  CHECK(priority_queue_pop(&pq) == &values[2]);
  CHECK(isEmpty_priority_queue(&pq));

  priority_queue_clear(&pq);
}

static void test_null_element(void) {
  int value = 4;
  priority_queue pq = priority_queue_init(2, compare_nullable_ints);

  priority_queue_push(&pq, &value);
  priority_queue_push(&pq, NULL);
  CHECK(!isEmpty_priority_queue(&pq));
  CHECK(priority_queue_peek(&pq) == NULL);
  CHECK(priority_queue_pop(&pq) == NULL);
  CHECK(!isEmpty_priority_queue(&pq));
  CHECK(priority_queue_pop(&pq) == &value);
  CHECK(isEmpty_priority_queue(&pq));

  priority_queue_clear(&pq);
}

static void test_clear_and_ownership(void) {
  int values[] = {3, 1, 2};
  priority_queue pq = priority_queue_init(2, compare_ints);
  for (size_t i = 0; i < 3; ++i)
    priority_queue_push(&pq, &values[i]);

  priority_queue_clear(&pq);
  CHECK(pq.data == NULL);
  CHECK(pq.size == 0);
  CHECK(pq.capacity == 0);
  CHECK(pq.compare == NULL);
  CHECK(values[0] == 3);
  CHECK(values[1] == 1);
  CHECK(values[2] == 2);

  priority_queue_clear(&pq);
  CHECK(pq.data == NULL);
}

static void dies_init_zero_capacity(void) {
  (void)priority_queue_init(0, compare_ints);
}

static void dies_init_without_comparator(void) {
  (void)priority_queue_init(4, NULL);
}

static void test_invalid_initialization(void) {
  check_exits_with_failure(dies_init_zero_capacity,
                           "zero-capacity priority queue");
  check_exits_with_failure(dies_init_without_comparator,
                           "priority queue without comparator");
}

int main(void) {
  test_initialization();
  test_push_and_peek();
  test_pop_sorted_order();
  test_automatic_growth();
  test_custom_priority();
  test_interleaved_operations();
  test_null_element();
  test_clear_and_ownership();
  test_invalid_initialization();

  if (tests_failed != 0) {
    fprintf(stderr, "%u/%u tests failed\n", tests_failed, tests_run);
    return EXIT_FAILURE;
  }
  printf("All %u priority-queue tests passed\n", tests_run);
  return EXIT_SUCCESS;
}
