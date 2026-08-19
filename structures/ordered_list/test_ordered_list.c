#include "ordered_list.h"

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

/* A distinct function with equivalent behavior, used for incompatibility. */
static int compare_ints_alternative(const elem left, const elem right) {
  const int a = *(const int *)left;
  const int b = *(const int *)right;
  return (a > b) - (a < b);
}

static void clear_ordered_list(ordered_list *ol) {
  ordered_list_clear(ol);
}

static void check_values(const ordered_list *ol, const int expected[],
                         size_t count) {
  pos current = ordered_list_first(ol);

  CHECK(ol->data.size == count);
  for (size_t i = 0; i < count; ++i) {
    CHECK(current != NULL);
    if (current == NULL)
      return;
    CHECK(*(int *)ordered_list_read(ol, current) == expected[i]);
    current = ordered_list_next(ol, current);
  }
  CHECK(current == NULL);
}

static void test_empty_ordered_list(void) {
  ordered_list ol = ordered_list_init(compare_ints);

  CHECK(ol.compare == compare_ints);
  CHECK(ol.data.size == 0);
  CHECK(ol.data.head == NULL);
  CHECK(isEmpty_ordered_list(&ol));
  CHECK(ordered_list_first(&ol) == NULL);
  CHECK(ordered_list_end(&ol, ordered_list_first(&ol)));
  CHECK(ordered_list_previous(&ol, NULL) == NULL);
}

static void test_clear(void) {
  int values[] = {3, 1, 2};
  ordered_list ol = ordered_list_init(compare_ints);

  ordered_list_clear(&ol);
  CHECK(isEmpty_ordered_list(&ol));
  CHECK(ol.compare == compare_ints);

  for (size_t i = 0; i < 3; ++i)
    ordered_list_insert(&ol, &values[i]);
  ordered_list_clear(&ol);
  CHECK(ol.data.head == NULL);
  CHECK(ol.data.size == 0);
  CHECK(ol.compare == compare_ints);
  CHECK(values[0] == 3);
  CHECK(values[1] == 1);
  CHECK(values[2] == 2);

  ordered_list_insert(&ol, &values[0]);
  CHECK(ordered_list_search(&ol, &values[0]));
  ordered_list_clear(&ol);
}

static void test_insert_maintains_order(void) {
  int values[] = {5, -2, 8, 1, 3, 0};
  const int expected[] = {-2, 0, 1, 3, 5, 8};
  ordered_list ol = ordered_list_init(compare_ints);

  for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
    ordered_list_insert(&ol, &values[i]);

  CHECK(!isEmpty_ordered_list(&ol));
  check_values(&ol, expected, sizeof(expected) / sizeof(expected[0]));
  clear_ordered_list(&ol);
}

static void test_insert_sorted_and_reverse_inputs(void) {
  int sorted_values[] = {1, 2, 3, 4};
  int reverse_values[] = {4, 3, 2, 1};
  const int expected[] = {1, 2, 3, 4};
  ordered_list sorted = ordered_list_init(compare_ints);
  ordered_list reverse = ordered_list_init(compare_ints);

  for (size_t i = 0; i < 4; ++i) {
    ordered_list_insert(&sorted, &sorted_values[i]);
    ordered_list_insert(&reverse, &reverse_values[i]);
  }

  check_values(&sorted, expected, 4);
  check_values(&reverse, expected, 4);
  clear_ordered_list(&sorted);
  clear_ordered_list(&reverse);
}

static void test_duplicate_values(void) {
  int values[] = {2, 1, 2, 1, 2};
  const int expected[] = {1, 1, 2, 2, 2};
  ordered_list ol = ordered_list_init(compare_ints);

  for (size_t i = 0; i < 5; ++i)
    ordered_list_insert(&ol, &values[i]);

  check_values(&ol, expected, 5);
  clear_ordered_list(&ol);
}

static void test_deduplicate(void) {
  int values[] = {3, 1, 2, 1, 3, 2, 2};
  const int expected[] = {1, 2, 3};
  ordered_list ol = ordered_list_init(compare_ints);

  for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
    ordered_list_insert(&ol, &values[i]);

  ordered_list_deduplicate(&ol);
  check_values(&ol, expected, sizeof(expected) / sizeof(expected[0]));
  clear_ordered_list(&ol);
}

static void test_deduplicate_edge_cases(void) {
  ordered_list empty = ordered_list_init(compare_ints);
  ordered_list_deduplicate(&empty);
  CHECK(isEmpty_ordered_list(&empty));
  CHECK(empty.data.size == 0);

  int unique_values[] = {3, 1, 2};
  const int unique_expected[] = {1, 2, 3};
  ordered_list unique = ordered_list_init(compare_ints);
  for (size_t i = 0; i < 3; ++i)
    ordered_list_insert(&unique, &unique_values[i]);
  ordered_list_deduplicate(&unique);
  check_values(&unique, unique_expected, 3);
  clear_ordered_list(&unique);

  int equal_values[] = {7, 7, 7, 7};
  const int equal_expected[] = {7};
  ordered_list all_equal = ordered_list_init(compare_ints);
  for (size_t i = 0; i < 4; ++i)
    ordered_list_insert(&all_equal, &equal_values[i]);
  ordered_list_deduplicate(&all_equal);
  check_values(&all_equal, equal_expected, 1);
  clear_ordered_list(&all_equal);

  int repeated = 9;
  const int repeated_expected[] = {9};
  ordered_list same_pointer = ordered_list_init(compare_ints);
  for (size_t i = 0; i < 3; ++i)
    ordered_list_insert(&same_pointer, &repeated);
  ordered_list_deduplicate(&same_pointer);
  check_values(&same_pointer, repeated_expected, 1);
  clear_ordered_list(&same_pointer);
}

static void test_navigation(void) {
  int values[] = {30, 10, 20};
  ordered_list ol = ordered_list_init(compare_ints);

  for (size_t i = 0; i < 3; ++i)
    ordered_list_insert(&ol, &values[i]);

  pos first = ordered_list_first(&ol);
  pos second = ordered_list_next(&ol, first);
  pos third = ordered_list_next(&ol, second);
  CHECK(*(int *)ordered_list_read(&ol, first) == 10);
  CHECK(*(int *)ordered_list_read(&ol, second) == 20);
  CHECK(*(int *)ordered_list_read(&ol, third) == 30);
  CHECK(ordered_list_previous(&ol, first) == NULL);
  CHECK(ordered_list_previous(&ol, second) == first);
  CHECK(ordered_list_previous(&ol, third) == second);
  CHECK(ordered_list_end(&ol, ordered_list_next(&ol, third)));

  clear_ordered_list(&ol);
}

static void test_remove_head_middle_and_tail(void) {
  int values[] = {1, 2, 3, 4};
  const int after_middle[] = {1, 3, 4};
  const int after_head[] = {3, 4};
  const int after_tail[] = {3};
  ordered_list ol = ordered_list_init(compare_ints);

  for (size_t i = 0; i < 4; ++i)
    ordered_list_insert(&ol, &values[i]);

  pos middle = ordered_list_next(&ol, ordered_list_first(&ol));
  ordered_list_remove(&ol, middle);
  check_values(&ol, after_middle, 3);

  ordered_list_remove(&ol, ordered_list_first(&ol));
  check_values(&ol, after_head, 2);

  pos tail = ordered_list_next(&ol, ordered_list_first(&ol));
  ordered_list_remove(&ol, tail);
  check_values(&ol, after_tail, 1);

  ordered_list_remove(&ol, ordered_list_first(&ol));
  CHECK(isEmpty_ordered_list(&ol));
  CHECK(ol.data.size == 0);
  CHECK(ol.data.head == NULL);
}

static void test_fusion(void) {
  int left_values[] = {5, 1, 3};
  int right_values[] = {6, 2, 3, 4};
  const int expected[] = {1, 2, 3, 3, 4, 5, 6};
  ordered_list left = ordered_list_init(compare_ints);
  ordered_list right = ordered_list_init(compare_ints);

  for (size_t i = 0; i < 3; ++i)
    ordered_list_insert(&left, &left_values[i]);
  for (size_t i = 0; i < 4; ++i)
    ordered_list_insert(&right, &right_values[i]);

  ordered_list result = ordered_list_fusion(&left, &right);
  CHECK(result.compare == compare_ints);
  check_values(&result, expected, 7);
  CHECK(left.data.size == 3);
  CHECK(right.data.size == 4);

  clear_ordered_list(&result);
  clear_ordered_list(&left);
  clear_ordered_list(&right);
}

static void test_fusion_with_empty_lists(void) {
  int values[] = {3, 1, 2};
  const int expected[] = {1, 2, 3};
  ordered_list empty = ordered_list_init(compare_ints);
  ordered_list populated = ordered_list_init(compare_ints);

  for (size_t i = 0; i < 3; ++i)
    ordered_list_insert(&populated, &values[i]);

  ordered_list result = ordered_list_fusion(&empty, &populated);
  check_values(&result, expected, 3);

  ordered_list both_empty = ordered_list_fusion(&empty, &empty);
  CHECK(isEmpty_ordered_list(&both_empty));
  CHECK(both_empty.compare == compare_ints);

  clear_ordered_list(&result);
  clear_ordered_list(&populated);
}

static void test_search(void) {
  int values[] = {8, -3, 14, 0, 8};
  int smallest_key = -3;
  int middle_key = 8;
  int largest_key = 14;
  int below_range = -20;
  int between_values = 9;
  int above_range = 50;
  ordered_list ol = ordered_list_init(compare_ints);

  CHECK(!ordered_list_search(&ol, &middle_key));

  for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
    ordered_list_insert(&ol, &values[i]);

  pos original_head = ol.data.head;
  size_t original_size = ol.data.size;
  CHECK(ordered_list_search(&ol, &smallest_key));
  CHECK(ordered_list_search(&ol, &middle_key));
  CHECK(ordered_list_search(&ol, &largest_key));
  CHECK(!ordered_list_search(&ol, &below_range));
  CHECK(!ordered_list_search(&ol, &between_values));
  CHECK(!ordered_list_search(&ol, &above_range));
  CHECK(ol.data.head == original_head);
  CHECK(ol.data.size == original_size);

  clear_ordered_list(&ol);
}

static void dies_next_at_end(void) {
  ordered_list ol = ordered_list_init(compare_ints);
  (void)ordered_list_next(&ol, NULL);
}

static void dies_read_empty(void) {
  ordered_list ol = ordered_list_init(compare_ints);
  (void)ordered_list_read(&ol, NULL);
}

static void dies_remove_empty(void) {
  ordered_list ol = ordered_list_init(compare_ints);
  ordered_list_remove(&ol, NULL);
}

static void dies_read_end(void) {
  int value = 1;
  ordered_list ol = ordered_list_init(compare_ints);
  ordered_list_insert(&ol, &value);
  (void)ordered_list_read(&ol, NULL);
}

static void dies_remove_end(void) {
  int value = 1;
  ordered_list ol = ordered_list_init(compare_ints);
  ordered_list_insert(&ol, &value);
  ordered_list_remove(&ol, NULL);
}

static void dies_remove_foreign_position(void) {
  int value = 1;
  node foreign = {.value = &value, .next = NULL};
  ordered_list ol = ordered_list_init(compare_ints);
  ordered_list_insert(&ol, &value);
  ordered_list_remove(&ol, &foreign);
}

static void dies_fusion_with_different_comparators(void) {
  ordered_list left = ordered_list_init(compare_ints);
  ordered_list right = ordered_list_init(compare_ints_alternative);
  (void)ordered_list_fusion(&left, &right);
}

static void test_invalid_operations(void) {
  check_exits_with_failure(dies_next_at_end, "ordered_list_next at end");
  check_exits_with_failure(dies_read_empty, "ordered_list_read on empty list");
  check_exits_with_failure(dies_remove_empty,
                           "ordered_list_remove on empty list");
  check_exits_with_failure(dies_read_end, "ordered_list_read at end");
  check_exits_with_failure(dies_remove_end, "ordered_list_remove at end");
  check_exits_with_failure(dies_remove_foreign_position,
                           "ordered_list_remove at foreign position");
  check_exits_with_failure(dies_fusion_with_different_comparators,
                           "fusion with different comparators");
}

int main(void) {
  test_empty_ordered_list();
  test_clear();
  test_insert_maintains_order();
  test_insert_sorted_and_reverse_inputs();
  test_duplicate_values();
  test_deduplicate();
  test_deduplicate_edge_cases();
  test_navigation();
  test_remove_head_middle_and_tail();
  test_fusion();
  test_fusion_with_empty_lists();
  test_search();
  test_invalid_operations();

  if (tests_failed != 0) {
    fprintf(stderr, "%u/%u tests failed\n", tests_failed, tests_run);
    return EXIT_FAILURE;
  }

  printf("All %u ordered-list tests passed\n", tests_run);
  return EXIT_SUCCESS;
}
