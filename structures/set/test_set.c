#include "set.h"

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

static size_t hash_int(const elem value) {
  return (size_t)*(const int *)value;
}

static size_t hash_int_alternative(const elem value) {
  return (size_t)*(const int *)value;
}

static size_t hash_constant(const elem value) {
  (void)value;
  return 0;
}

static bool equal_int(const elem left, const elem right) {
  return *(const int *)left == *(const int *)right;
}

static bool equal_int_alternative(const elem left, const elem right) {
  return *(const int *)left == *(const int *)right;
}

static void check_members(const set *s, int values[], size_t count,
                          bool expected) {
  for (size_t i = 0; i < count; ++i)
    CHECK(set_contains(s, &values[i]) == expected);
}

static void test_initialization(void) {
  set s = set_init(8, hash_int, equal_int);

  CHECK(s.slots != NULL);
  CHECK(s.size == 0);
  CHECK(s.capacity == 8);
  CHECK(s.hash == hash_int);
  CHECK(s.equal == equal_int);
  CHECK(isEmpty_set(&s));
  for (size_t i = 0; i < s.capacity; ++i) {
    CHECK(s.slots[i].state == SLOT_EMPTY);
    CHECK(s.slots[i].value == NULL);
  }

  set_clear(&s);
}

static void test_insert_contains_and_duplicates(void) {
  int values[] = {4, 12, 7};
  int equivalent_four = 4;
  int missing = 99;
  set s = set_init(8, hash_int, equal_int);

  CHECK(!set_contains(&s, &missing));
  for (size_t i = 0; i < 3; ++i)
    set_insert(&s, &values[i]);
  CHECK(s.size == 3);
  CHECK(!isEmpty_set(&s));
  check_members(&s, values, 3, true);
  CHECK(!set_contains(&s, &missing));

  set_insert(&s, &equivalent_four);
  CHECK(s.size == 3);
  CHECK(set_contains(&s, &equivalent_four));

  set_clear(&s);
}

static void test_collisions_and_full_capacity(void) {
  int values[] = {0, 1, 2, 3, 4, 5, 6, 7};
  set s = set_init(8, hash_constant, equal_int);

  for (size_t i = 0; i < 8; ++i)
    set_insert(&s, &values[i]);
  CHECK(s.size == 8);
  check_members(&s, values, 8, true);

  set_clear(&s);
}

static void test_remove_and_tombstone_reuse(void) {
  int values[] = {1, 2, 3};
  int missing = 40;
  int replacement = 4;
  set s = set_init(3, hash_constant, equal_int);

  for (size_t i = 0; i < 3; ++i)
    set_insert(&s, &values[i]);
  set_remove(&s, &values[1]);
  CHECK(s.size == 2);
  CHECK(!set_contains(&s, &values[1]));
  CHECK(set_contains(&s, &values[0]));
  CHECK(set_contains(&s, &values[2]));

  set_remove(&s, &missing);
  CHECK(s.size == 2);
  set_insert(&s, &replacement);
  CHECK(s.size == 3);
  CHECK(set_contains(&s, &replacement));

  set_clear(&s);
}

static void test_remove_all_and_reuse(void) {
  int values[] = {1, 2, 3};
  set s = set_init(4, hash_int, equal_int);

  for (size_t i = 0; i < 3; ++i)
    set_insert(&s, &values[i]);
  for (size_t i = 0; i < 3; ++i)
    set_remove(&s, &values[i]);
  CHECK(isEmpty_set(&s));
  CHECK(s.size == 0);

  set_insert(&s, &values[2]);
  CHECK(set_contains(&s, &values[2]));
  CHECK(s.size == 1);
  set_clear(&s);
}

static void test_clear(void) {
  int values[] = {1, 2};
  set s = set_init(4, hash_int, equal_int);
  set_insert(&s, &values[0]);
  set_insert(&s, &values[1]);

  set_clear(&s);
  CHECK(s.slots == NULL);
  CHECK(s.size == 0);
  CHECK(s.capacity == 0);
  CHECK(s.hash == NULL);
  CHECK(s.equal == NULL);
  CHECK(values[0] == 1);
  CHECK(values[1] == 2);

  set_clear(&s);
  CHECK(s.slots == NULL);
}

static void fill_set(set *s, int values[], size_t count) {
  for (size_t i = 0; i < count; ++i)
    set_insert(s, &values[i]);
}

static void test_set_operations(void) {
  int a_values[] = {1, 2, 3};
  int b_values[] = {3, 4, 5};
  int union_values[] = {1, 2, 3, 4, 5};
  int intersection_values[] = {3};
  int difference_values[] = {1, 2};
  set a = set_init(5, hash_int, equal_int);
  set b = set_init(5, hash_int, equal_int);
  fill_set(&a, a_values, 3);
  fill_set(&b, b_values, 3);

  set united = set_union(&a, &b);
  CHECK(united.size == 5);
  check_members(&united, union_values, 5, true);

  set intersected = set_intersect(&a, &b);
  CHECK(intersected.size == 1);
  check_members(&intersected, intersection_values, 1, true);
  CHECK(!set_contains(&intersected, &a_values[0]));
  CHECK(!set_contains(&intersected, &b_values[1]));

  set difference = set_difference(&a, &b);
  CHECK(difference.size == 2);
  check_members(&difference, difference_values, 2, true);
  CHECK(!set_contains(&difference, &a_values[2]));

  CHECK(a.size == 3);
  CHECK(b.size == 3);
  set_clear(&united);
  set_clear(&intersected);
  set_clear(&difference);
  set_clear(&a);
  set_clear(&b);
}

static void test_operations_with_empty_sets(void) {
  int values[] = {1, 2};
  set empty = set_init(3, hash_int, equal_int);
  set populated = set_init(3, hash_int, equal_int);
  fill_set(&populated, values, 2);

  set united = set_union(&empty, &populated);
  check_members(&united, values, 2, true);
  set intersected = set_intersect(&empty, &populated);
  CHECK(isEmpty_set(&intersected));
  set difference = set_difference(&populated, &empty);
  check_members(&difference, values, 2, true);

  set_clear(&united);
  set_clear(&intersected);
  set_clear(&difference);
  set_clear(&empty);
  set_clear(&populated);
}

static void dies_init_zero_capacity(void) {
  (void)set_init(0, hash_int, equal_int);
}

static void dies_init_without_hash(void) {
  (void)set_init(4, NULL, equal_int);
}

static void dies_init_without_equal(void) {
  (void)set_init(4, hash_int, NULL);
}

static void dies_insert_full(void) {
  int values[] = {1, 2};
  set s = set_init(1, hash_int, equal_int);
  set_insert(&s, &values[0]);
  set_insert(&s, &values[1]);
}

static void dies_union_incompatible_hash(void) {
  set a = set_init(2, hash_int, equal_int);
  set b = set_init(2, hash_int_alternative, equal_int);
  (void)set_union(&a, &b);
}

static void dies_intersect_incompatible_equal(void) {
  set a = set_init(2, hash_int, equal_int);
  set b = set_init(2, hash_int, equal_int_alternative);
  (void)set_intersect(&a, &b);
}

static void dies_difference_incompatible_hash(void) {
  set a = set_init(2, hash_int, equal_int);
  set b = set_init(2, hash_int_alternative, equal_int);
  (void)set_difference(&a, &b);
}

static void test_invalid_operations(void) {
  check_exits_with_failure(dies_init_zero_capacity, "zero-capacity set");
  check_exits_with_failure(dies_init_without_hash, "set without hash");
  check_exits_with_failure(dies_init_without_equal, "set without equality");
  check_exits_with_failure(dies_insert_full, "insertion into full set");
  check_exits_with_failure(dies_union_incompatible_hash,
                           "union with incompatible hash");
  check_exits_with_failure(dies_intersect_incompatible_equal,
                           "intersection with incompatible equality");
  check_exits_with_failure(dies_difference_incompatible_hash,
                           "difference with incompatible hash");
}

int main(void) {
  test_initialization();
  test_insert_contains_and_duplicates();
  test_collisions_and_full_capacity();
  test_remove_and_tombstone_reuse();
  test_remove_all_and_reuse();
  test_clear();
  test_set_operations();
  test_operations_with_empty_sets();
  test_invalid_operations();

  if (tests_failed != 0) {
    fprintf(stderr, "%u/%u tests failed\n", tests_failed, tests_run);
    return EXIT_FAILURE;
  }
  printf("All %u set tests passed\n", tests_run);
  return EXIT_SUCCESS;
}
