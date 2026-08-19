#include "stack.h"

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

static void test_empty_stack(void) {
  stack s = stack_init();

  CHECK(isEmpty_stack(&s));
  CHECK(s.data.size == 0);
  CHECK(s.data.head == NULL);
}

static void test_push_and_top(void) {
  int a = 10;
  int b = 20;
  stack s = stack_init();

  stack_push(&s, &a);
  CHECK(!isEmpty_stack(&s));
  CHECK(s.data.size == 1);
  CHECK(stack_top(&s) == &a);
  CHECK(s.data.size == 1);

  stack_push(&s, &b);
  CHECK(s.data.size == 2);
  CHECK(stack_top(&s) == &b);
  CHECK(s.data.size == 2);

  stack_clear(&s);
}

static void test_lifo_order(void) {
  int values[] = {1, 2, 3, 4, 5};
  stack s = stack_init();

  for (size_t i = 0; i < 5; ++i) {
    stack_push(&s, &values[i]);
    CHECK(stack_top(&s) == &values[i]);
    CHECK(s.data.size == i + 1);
  }

  for (size_t i = 5; i > 0; --i) {
    CHECK(stack_top(&s) == &values[i - 1]);
    CHECK(stack_pop(&s) == &values[i - 1]);
    CHECK(s.data.size == i - 1);
  }

  CHECK(isEmpty_stack(&s));
  CHECK(s.data.head == NULL);
}

static void test_interleaved_operations(void) {
  int a = 1;
  int b = 2;
  int c = 3;
  stack s = stack_init();

  stack_push(&s, &a);
  stack_push(&s, &b);
  CHECK(stack_pop(&s) == &b);
  stack_push(&s, &c);
  CHECK(stack_top(&s) == &c);
  CHECK(stack_pop(&s) == &c);
  CHECK(stack_pop(&s) == &a);
  CHECK(isEmpty_stack(&s));
}

static void test_null_element(void) {
  int value = 7;
  stack s = stack_init();

  stack_push(&s, &value);
  stack_push(&s, NULL);
  CHECK(!isEmpty_stack(&s));
  CHECK(stack_top(&s) == NULL);
  CHECK(stack_pop(&s) == NULL);
  CHECK(stack_top(&s) == &value);

  stack_clear(&s);
}

static void test_clear_and_reuse(void) {
  int values[] = {1, 2, 3};
  stack s = stack_init();

  stack_clear(&s);
  CHECK(isEmpty_stack(&s));

  for (size_t i = 0; i < 3; ++i)
    stack_push(&s, &values[i]);
  stack_clear(&s);
  CHECK(isEmpty_stack(&s));
  CHECK(s.data.size == 0);
  CHECK(s.data.head == NULL);

  /* Clear frees stack nodes, not caller-owned values. */
  CHECK(values[0] == 1);
  CHECK(values[1] == 2);
  CHECK(values[2] == 3);

  stack_clear(&s);
  stack_push(&s, &values[0]);
  CHECK(stack_pop(&s) == &values[0]);
  CHECK(isEmpty_stack(&s));
}

static void dies_pop_empty(void) {
  stack s = stack_init();
  (void)stack_pop(&s);
}

static void dies_top_empty(void) {
  stack s = stack_init();
  (void)stack_top(&s);
}

static void test_invalid_operations(void) {
  check_exits_with_failure(dies_pop_empty, "stack_pop on empty stack");
  check_exits_with_failure(dies_top_empty, "stack_top on empty stack");
}

int main(void) {
  test_empty_stack();
  test_push_and_top();
  test_lifo_order();
  test_interleaved_operations();
  test_null_element();
  test_clear_and_reuse();
  test_invalid_operations();

  if (tests_failed != 0) {
    fprintf(stderr, "%u/%u tests failed\n", tests_failed, tests_run);
    return EXIT_FAILURE;
  }

  printf("All %u stack tests passed\n", tests_run);
  return EXIT_SUCCESS;
}
