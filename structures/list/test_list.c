#include "list.h"

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
    /* errx/err diagnostics are expected and would only clutter test output. */
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

static void clear_list(list *l) {
  list_clear(l);
}

static int compare_ints(const elem left, const elem right) {
  const int a = *(const int *)left;
  const int b = *(const int *)right;
  return (a > b) - (a < b);
}

static void check_int_values(const list *l, const int expected[], size_t count) {
  pos current = list_first(l);

  CHECK(l->size == count);
  for (size_t i = 0; i < count; ++i) {
    CHECK(current != NULL);
    if (current == NULL)
      return;
    CHECK(*(int *)list_read(l, current) == expected[i]);
    current = list_next(l, current);
  }
  CHECK(current == NULL);
}

static void test_empty_list(void) {
  list l = list_init();

  CHECK(l.size == 0);
  CHECK(l.head == NULL);
  CHECK(isEmpty_list(&l));
  CHECK(list_first(&l) == NULL);
  CHECK(list_end(&l, list_first(&l)));
  CHECK(list_previous(&l, NULL) == NULL);
}

static void test_clear(void) {
  int values[] = {1, 2, 3};
  list l = list_init();

  list_clear(&l);
  CHECK(l.head == NULL);
  CHECK(l.size == 0);

  for (size_t i = 0; i < 3; ++i)
    list_insert(&l, &values[i], NULL);
  list_clear(&l);
  CHECK(l.head == NULL);
  CHECK(l.size == 0);
  CHECK(isEmpty_list(&l));

  /* Clearing nodes must not destroy caller-owned element values. */
  CHECK(values[0] == 1);
  CHECK(values[1] == 2);
  CHECK(values[2] == 3);

  list_clear(&l);
  CHECK(isEmpty_list(&l));

  list_insert(&l, &values[0], NULL);
  CHECK(l.size == 1);
  CHECK(list_read(&l, list_first(&l)) == &values[0]);
  list_clear(&l);
}

static void test_insert_at_head(void) {
  int a = 10;
  int b = 20;
  list l = list_init();

  list_insert(&l, &a, list_first(&l));
  CHECK(l.size == 1);
  CHECK(!isEmpty_list(&l));
  CHECK(list_read(&l, list_first(&l)) == &a);
  CHECK(list_next(&l, list_first(&l)) == NULL);

  list_insert(&l, &b, list_first(&l));
  CHECK(l.size == 2);
  CHECK(list_read(&l, list_first(&l)) == &b);
  CHECK(list_read(&l, list_next(&l, list_first(&l))) == &a);

  clear_list(&l);
}

static void test_insert_at_tail_and_middle(void) {
  int a = 1;
  int b = 2;
  int middle = 99;
  list l = list_init();

  list_insert(&l, &a, NULL);
  list_insert(&l, &b, NULL);
  pos second = list_next(&l, list_first(&l));
  list_insert(&l, &middle, second);

  pos first = list_first(&l);
  pos inserted = list_next(&l, first);
  CHECK(l.size == 3);
  CHECK(list_read(&l, first) == &a);
  CHECK(list_read(&l, inserted) == &middle);
  CHECK(list_read(&l, list_next(&l, inserted)) == &b);
  CHECK(list_previous(&l, first) == NULL);
  CHECK(list_previous(&l, inserted) == first);
  CHECK(list_previous(&l, second) == inserted);
  CHECK(list_previous(&l, NULL) == NULL);

  clear_list(&l);
}

static void test_write(void) {
  int old_value = 4;
  int new_value = 5;
  list l = list_init();

  list_insert(&l, &old_value, NULL);
  list_write(&l, &new_value, list_first(&l));
  CHECK(l.size == 1);
  CHECK(list_read(&l, list_first(&l)) == &new_value);

  clear_list(&l);
}

static void test_null_element_is_valid(void) {
  list l = list_init();

  list_insert(&l, NULL, NULL);
  CHECK(l.size == 1);
  CHECK(list_read(&l, list_first(&l)) == NULL);

  clear_list(&l);
}

static void test_remove_head_middle_and_tail(void) {
  int a = 1;
  int b = 2;
  int c = 3;
  list l = list_init();

  list_insert(&l, &a, NULL);
  list_insert(&l, &b, NULL);
  list_insert(&l, &c, NULL);

  pos middle = list_next(&l, list_first(&l));
  list_remove(&l, middle);
  CHECK(l.size == 2);
  CHECK(list_read(&l, list_first(&l)) == &a);
  CHECK(list_read(&l, list_next(&l, list_first(&l))) == &c);

  pos tail = list_next(&l, list_first(&l));
  list_remove(&l, tail);
  CHECK(l.size == 1);
  CHECK(list_next(&l, list_first(&l)) == NULL);

  list_remove(&l, list_first(&l));
  CHECK(l.size == 0);
  CHECK(l.head == NULL);
  CHECK(isEmpty_list(&l));
}

static void test_natural_mergesort(void) {
  int values[] = {7, 2, 2, -4, 10, 3, 1, 8};
  const int expected[] = {-4, 1, 2, 2, 3, 7, 8, 10};
  list l = list_init();

  for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
    list_insert(&l, &values[i], NULL);

  list_natural_mergesort(&l, compare_ints);
  check_int_values(&l, expected, sizeof(expected) / sizeof(expected[0]));
  clear_list(&l);
}

static void test_natural_mergesort_edge_cases(void) {
  list empty = list_init();
  list_natural_mergesort(&empty, compare_ints);
  CHECK(empty.head == NULL);
  CHECK(empty.size == 0);

  int one = 1;
  list single = list_init();
  list_insert(&single, &one, NULL);
  pos original_node = single.head;
  list_natural_mergesort(&single, compare_ints);
  CHECK(single.head == original_node);
  CHECK(single.size == 1);
  clear_list(&single);

  int sorted_values[] = {1, 2, 3, 4};
  const int sorted_expected[] = {1, 2, 3, 4};
  list sorted = list_init();
  for (size_t i = 0; i < 4; ++i)
    list_insert(&sorted, &sorted_values[i], NULL);
  list_natural_mergesort(&sorted, compare_ints);
  check_int_values(&sorted, sorted_expected, 4);
  clear_list(&sorted);

  int reverse_values[] = {5, 4, 3, 2, 1};
  const int reverse_expected[] = {1, 2, 3, 4, 5};
  list reverse = list_init();
  for (size_t i = 0; i < 5; ++i)
    list_insert(&reverse, &reverse_values[i], NULL);
  list_natural_mergesort(&reverse, compare_ints);
  check_int_values(&reverse, reverse_expected, 5);
  clear_list(&reverse);
}

typedef struct {
  int key;
  char original_order;
} item;

static int compare_items(const elem left, const elem right) {
  const item *a = left;
  const item *b = right;
  return (a->key > b->key) - (a->key < b->key);
}

static void test_deduplicate(void) {
  int values[] = {3, 1, 3, 2, 1, 1, 2};
  const int expected[] = {3, 1, 2};
  list l = list_init();

  for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
    list_insert(&l, &values[i], NULL);

  list_deduplicate(&l, compare_ints);
  check_int_values(&l, expected, sizeof(expected) / sizeof(expected[0]));
  clear_list(&l);
}

static void test_deduplicate_edge_cases(void) {
  list empty = list_init();
  list_deduplicate(&empty, compare_ints);
  CHECK(isEmpty_list(&empty));

  int unique_values[] = {3, 1, 2};
  const int unique_expected[] = {3, 1, 2};
  list unique = list_init();
  for (size_t i = 0; i < 3; ++i)
    list_insert(&unique, &unique_values[i], NULL);
  list_deduplicate(&unique, compare_ints);
  check_int_values(&unique, unique_expected, 3);
  clear_list(&unique);

  item values[] = {{2, 'a'}, {1, 'b'}, {2, 'c'}, {1, 'd'}, {2, 'e'}};
  list equivalent = list_init();
  for (size_t i = 0; i < 5; ++i)
    list_insert(&equivalent, &values[i], NULL);
  list_deduplicate(&equivalent, compare_items);
  CHECK(equivalent.size == 2);
  CHECK(list_read(&equivalent, list_first(&equivalent)) == &values[0]);
  CHECK(list_read(&equivalent,
                  list_next(&equivalent, list_first(&equivalent))) ==
        &values[1]);
  clear_list(&equivalent);
}

static void test_natural_mergesort_is_stable(void) {
  item values[] = {{2, 'a'}, {1, 'b'}, {2, 'c'}, {1, 'd'}, {2, 'e'}};
  const char expected_order[] = {'b', 'd', 'a', 'c', 'e'};
  list l = list_init();

  for (size_t i = 0; i < 5; ++i)
    list_insert(&l, &values[i], NULL);
  list_natural_mergesort(&l, compare_items);

  pos current = list_first(&l);
  for (size_t i = 0; i < 5; ++i) {
    CHECK(current != NULL);
    CHECK(((item *)list_read(&l, current))->original_order == expected_order[i]);
    current = list_next(&l, current);
  }
  clear_list(&l);
}

static void test_list_fusion(void) {
  int left_values[] = {1, 3, 5};
  int right_values[] = {2, 3, 4, 6};
  const int expected[] = {1, 2, 3, 3, 4, 5, 6};
  list left = list_init();
  list right = list_init();

  for (size_t i = 0; i < 3; ++i)
    list_insert(&left, &left_values[i], NULL);
  for (size_t i = 0; i < 4; ++i)
    list_insert(&right, &right_values[i], NULL);

  list result = list_fusion(&left, &right, compare_ints);
  check_int_values(&result, expected, 7);
  CHECK(left.size == 3);
  CHECK(right.size == 4);

  clear_list(&result);
  clear_list(&left);
  clear_list(&right);
}

static void test_list_fusion_with_empty_lists(void) {
  int values[] = {1, 2, 3};
  const int expected[] = {1, 2, 3};
  list empty = list_init();
  list populated = list_init();

  for (size_t i = 0; i < 3; ++i)
    list_insert(&populated, &values[i], NULL);

  list left_empty = list_fusion(&empty, &populated, compare_ints);
  list right_empty = list_fusion(&populated, &empty, compare_ints);
  list both_empty = list_fusion(&empty, &empty, compare_ints);
  check_int_values(&left_empty, expected, 3);
  check_int_values(&right_empty, expected, 3);
  CHECK(isEmpty_list(&both_empty));
  CHECK(populated.size == 3);

  clear_list(&left_empty);
  clear_list(&right_empty);
  clear_list(&both_empty);
  clear_list(&populated);
}

static void test_search(void) {
  int values[] = {8, -3, 14, 0, 8};
  int first_key = 8;
  int middle_key = 14;
  int last_key = 0;
  int missing_key = 99;
  list l = list_init();

  CHECK(!list_search(&l, &first_key, compare_ints));

  for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
    list_insert(&l, &values[i], NULL);

  pos original_head = l.head;
  size_t original_size = l.size;
  CHECK(list_search(&l, &first_key, compare_ints));
  CHECK(list_search(&l, &middle_key, compare_ints));
  CHECK(list_search(&l, &last_key, compare_ints));
  CHECK(!list_search(&l, &missing_key, compare_ints));
  CHECK(l.head == original_head);
  CHECK(l.size == original_size);

  clear_list(&l);
}

static void test_search_uses_comparator_equality(void) {
  item stored = {.key = 4, .original_order = 'a'};
  item equivalent_key = {.key = 4, .original_order = 'z'};
  item missing_key = {.key = 5, .original_order = 'a'};
  list l = list_init();

  list_insert(&l, &stored, NULL);
  CHECK(list_search(&l, &equivalent_key, compare_items));
  CHECK(!list_search(&l, &missing_key, compare_items));

  clear_list(&l);
}

static void dies_next_at_end(void) {
  list l = list_init();
  (void)list_next(&l, NULL);
}

static void dies_read_empty(void) {
  list l = list_init();
  (void)list_read(&l, NULL);
}

static void dies_write_empty(void) {
  list l = list_init();
  list_write(&l, NULL, NULL);
}

static void dies_remove_empty(void) {
  list l = list_init();
  list_remove(&l, NULL);
}

static void dies_read_end(void) {
  int value = 1;
  list l = list_init();
  list_insert(&l, &value, NULL);
  (void)list_read(&l, NULL);
}

static void dies_write_end(void) {
  int value = 1;
  list l = list_init();
  list_insert(&l, &value, NULL);
  list_write(&l, &value, NULL);
}

static void dies_remove_end(void) {
  int value = 1;
  list l = list_init();
  list_insert(&l, &value, NULL);
  list_remove(&l, NULL);
}

static void dies_insert_at_foreign_position(void) {
  int value = 1;
  node foreign = {.value = &value, .next = NULL};
  list l = list_init();
  list_insert(&l, &value, NULL);
  list_insert(&l, &value, &foreign);
}

static void dies_remove_foreign_position(void) {
  int value = 1;
  node foreign = {.value = &value, .next = NULL};
  list l = list_init();
  list_insert(&l, &value, NULL);
  list_remove(&l, &foreign);
}

static void dies_sort_without_comparator(void) {
  list l = list_init();
  list_natural_mergesort(&l, NULL);
}

static void dies_deduplicate_without_comparator(void) {
  list l = list_init();
  list_deduplicate(&l, NULL);
}

static void dies_fusion_without_comparator(void) {
  list left = list_init();
  list right = list_init();
  (void)list_fusion(&left, &right, NULL);
}

static void test_invalid_operations(void) {
  check_exits_with_failure(dies_next_at_end, "list_next at end");
  check_exits_with_failure(dies_read_empty, "list_read on empty list");
  check_exits_with_failure(dies_write_empty, "list_write on empty list");
  check_exits_with_failure(dies_remove_empty, "list_remove on empty list");
  check_exits_with_failure(dies_read_end, "list_read at end");
  check_exits_with_failure(dies_write_end, "list_write at end");
  check_exits_with_failure(dies_remove_end, "list_remove at end");
  check_exits_with_failure(dies_insert_at_foreign_position,
                           "list_insert at foreign position");
  check_exits_with_failure(dies_remove_foreign_position,
                           "list_remove at foreign position");
  check_exits_with_failure(dies_sort_without_comparator,
                           "list_natural_mergesort without comparator");
  check_exits_with_failure(dies_deduplicate_without_comparator,
                           "list_deduplicate without comparator");
  check_exits_with_failure(dies_fusion_without_comparator,
                           "list_fusion without comparator");
}

int main(void) {
  test_empty_list();
  test_clear();
  test_insert_at_head();
  test_insert_at_tail_and_middle();
  test_write();
  test_null_element_is_valid();
  test_remove_head_middle_and_tail();
  test_deduplicate();
  test_deduplicate_edge_cases();
  test_natural_mergesort();
  test_natural_mergesort_edge_cases();
  test_natural_mergesort_is_stable();
  test_list_fusion();
  test_list_fusion_with_empty_lists();
  test_search();
  test_search_uses_comparator_equality();
  test_invalid_operations();

  if (tests_failed != 0) {
    fprintf(stderr, "%u/%u tests failed\n", tests_failed, tests_run);
    return EXIT_FAILURE;
  }

  printf("All %u tests passed\n", tests_run);
  return EXIT_SUCCESS;
}
