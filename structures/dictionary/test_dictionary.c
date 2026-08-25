#include "dictionary.h"

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

static size_t hash_constant(const elem value) {
  (void)value;
  return 0;
}

static bool equal_int(const elem left, const elem right) {
  return *(const int *)left == *(const int *)right;
}

static void test_initialization(void) {
  dictionary d = dictionary_init(8, hash_int, equal_int);

  CHECK(d.slots != NULL);
  CHECK(d.size == 0);
  CHECK(d.capacity == 8);
  CHECK(d.hash == hash_int);
  CHECK(d.equal == equal_int);
  CHECK(isEmpty_dictionary(&d));
  for (size_t i = 0; i < d.capacity; ++i) {
    CHECK(d.slots[i].state == SLOT_EMPTY);
    CHECK(d.slots[i].key == NULL);
    CHECK(d.slots[i].value == NULL);
  }

  dictionary_clear(&d);
}

static void test_insert_contains_and_get(void) {
  int keys[] = {4, 12, 7};
  int values[] = {40, 120, 70};
  int equivalent_key = 4;
  int missing = 99;
  dictionary d = dictionary_init(8, hash_int, equal_int);

  CHECK(!dictionary_contains(&d, &missing));
  for (size_t i = 0; i < 3; ++i)
    dictionary_insert(&d, &keys[i], &values[i]);

  CHECK(d.size == 3);
  CHECK(!isEmpty_dictionary(&d));
  for (size_t i = 0; i < 3; ++i) {
    CHECK(dictionary_contains(&d, &keys[i]));
    CHECK(dictionary_get(&d, &keys[i]) == &values[i]);
  }
  CHECK(dictionary_contains(&d, &equivalent_key));
  CHECK(dictionary_get(&d, &equivalent_key) == &values[0]);
  CHECK(!dictionary_contains(&d, &missing));

  dictionary_clear(&d);
}

static void test_insert_updates_existing_key(void) {
  int original_key = 5;
  int equivalent_key = 5;
  int original_value = 10;
  int replacement_value = 20;
  dictionary d = dictionary_init(4, hash_int, equal_int);

  dictionary_insert(&d, &original_key, &original_value);
  dictionary_insert(&d, &equivalent_key, &replacement_value);
  CHECK(d.size == 1);
  CHECK(dictionary_get(&d, &original_key) == &replacement_value);
  CHECK(dictionary_get(&d, &equivalent_key) == &replacement_value);

  dictionary_clear(&d);
}

static void test_collisions_and_full_capacity(void) {
  int keys[] = {1, 2, 3, 4, 5, 6, 7, 8};
  int values[] = {10, 20, 30, 40, 50, 60, 70, 80};
  dictionary d = dictionary_init(8, hash_constant, equal_int);

  for (size_t i = 0; i < 8; ++i)
    dictionary_insert(&d, &keys[i], &values[i]);
  CHECK(d.size == 8);
  for (size_t i = 0; i < 8; ++i) {
    CHECK(dictionary_contains(&d, &keys[i]));
    CHECK(dictionary_get(&d, &keys[i]) == &values[i]);
  }

  dictionary_clear(&d);
}

static void test_remove_and_tombstone_reuse(void) {
  int keys[] = {1, 2, 3};
  int values[] = {10, 20, 30};
  int missing = 50;
  int replacement_key = 4;
  int replacement_value = 40;
  dictionary d = dictionary_init(3, hash_constant, equal_int);

  for (size_t i = 0; i < 3; ++i)
    dictionary_insert(&d, &keys[i], &values[i]);
  dictionary_remove(&d, &keys[1]);
  CHECK(d.size == 2);
  CHECK(!dictionary_contains(&d, &keys[1]));
  CHECK(dictionary_get(&d, &keys[0]) == &values[0]);
  CHECK(dictionary_get(&d, &keys[2]) == &values[2]);

  dictionary_remove(&d, &missing);
  CHECK(d.size == 2);
  dictionary_insert(&d, &replacement_key, &replacement_value);
  CHECK(d.size == 3);
  CHECK(dictionary_get(&d, &replacement_key) == &replacement_value);

  dictionary_clear(&d);
}

static void test_remove_all_and_reuse(void) {
  int keys[] = {1, 2, 3};
  int values[] = {10, 20, 30};
  dictionary d = dictionary_init(4, hash_int, equal_int);

  for (size_t i = 0; i < 3; ++i)
    dictionary_insert(&d, &keys[i], &values[i]);
  for (size_t i = 0; i < 3; ++i)
    dictionary_remove(&d, &keys[i]);
  CHECK(isEmpty_dictionary(&d));
  CHECK(d.size == 0);

  dictionary_insert(&d, &keys[2], &values[2]);
  CHECK(dictionary_get(&d, &keys[2]) == &values[2]);
  CHECK(d.size == 1);
  dictionary_clear(&d);
}

static void test_null_value(void) {
  int key = 1;
  dictionary d = dictionary_init(2, hash_int, equal_int);

  dictionary_insert(&d, &key, NULL);
  CHECK(dictionary_contains(&d, &key));
  CHECK(dictionary_get(&d, &key) == NULL);
  CHECK(d.size == 1);

  dictionary_clear(&d);
}

static void test_clear_and_ownership(void) {
  int key = 1;
  int value = 10;
  dictionary d = dictionary_init(2, hash_int, equal_int);
  dictionary_insert(&d, &key, &value);

  dictionary_clear(&d);
  CHECK(d.slots == NULL);
  CHECK(d.size == 0);
  CHECK(d.capacity == 0);
  CHECK(d.hash == NULL);
  CHECK(d.equal == NULL);
  CHECK(key == 1);
  CHECK(value == 10);

  dictionary_clear(&d);
  CHECK(d.slots == NULL);
}

static void dies_init_zero_capacity(void) {
  (void)dictionary_init(0, hash_int, equal_int);
}

static void dies_init_without_hash(void) {
  (void)dictionary_init(4, NULL, equal_int);
}

static void dies_init_without_equal(void) {
  (void)dictionary_init(4, hash_int, NULL);
}

static void dies_insert_full(void) {
  int keys[] = {1, 2};
  int values[] = {10, 20};
  dictionary d = dictionary_init(1, hash_int, equal_int);
  dictionary_insert(&d, &keys[0], &values[0]);
  dictionary_insert(&d, &keys[1], &values[1]);
}

static void dies_get_empty(void) {
  int key = 1;
  dictionary d = dictionary_init(2, hash_int, equal_int);
  (void)dictionary_get(&d, &key);
}

static void dies_get_missing(void) {
  int key = 1;
  int missing = 2;
  int value = 10;
  dictionary d = dictionary_init(2, hash_int, equal_int);
  dictionary_insert(&d, &key, &value);
  (void)dictionary_get(&d, &missing);
}

static void dies_contains_after_clear(void) {
  int key = 1;
  dictionary d = dictionary_init(2, hash_int, equal_int);
  dictionary_clear(&d);
  (void)dictionary_contains(&d, &key);
}

static void dies_remove_after_clear(void) {
  int key = 1;
  dictionary d = dictionary_init(2, hash_int, equal_int);
  dictionary_clear(&d);
  dictionary_remove(&d, &key);
}

static void test_invalid_operations(void) {
  check_exits_with_failure(dies_init_zero_capacity,
                           "zero-capacity dictionary");
  check_exits_with_failure(dies_init_without_hash, "dictionary without hash");
  check_exits_with_failure(dies_init_without_equal,
                           "dictionary without equality");
  check_exits_with_failure(dies_insert_full,
                           "insertion into full dictionary");
  check_exits_with_failure(dies_get_empty, "get from empty dictionary");
  check_exits_with_failure(dies_get_missing, "get missing key");
  check_exits_with_failure(dies_contains_after_clear, "contains after clear");
  check_exits_with_failure(dies_remove_after_clear, "remove after clear");
}

int main(void) {
  test_initialization();
  test_insert_contains_and_get();
  test_insert_updates_existing_key();
  test_collisions_and_full_capacity();
  test_remove_and_tombstone_reuse();
  test_remove_all_and_reuse();
  test_null_value();
  test_clear_and_ownership();
  test_invalid_operations();

  if (tests_failed != 0) {
    fprintf(stderr, "%u/%u tests failed\n", tests_failed, tests_run);
    return EXIT_FAILURE;
  }
  printf("All %u dictionary tests passed\n", tests_run);
  return EXIT_SUCCESS;
}
