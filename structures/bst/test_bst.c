#include "bst.h"

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

static size_t validate_subtree(const bst *tree, b_pos node, b_pos expected_father,
                               elem lower, elem upper) {
  if (node == NULL)
    return 0;

  CHECK(node->father == expected_father);
  if (lower != NULL)
    CHECK(tree->data.compare(node->value, lower) > 0);
  if (upper != NULL)
    CHECK(tree->data.compare(node->value, upper) < 0);

  return 1 + validate_subtree(tree, node->left, node, lower, node->value) +
         validate_subtree(tree, node->right, node, node->value, upper);
}

static void check_bst_invariants(const bst *tree) {
  size_t reachable = validate_subtree(tree, tree->data.root, NULL, NULL, NULL);
  CHECK(reachable == tree->data.size);
  CHECK((tree->data.root == NULL) == (tree->data.size == 0));
  CHECK(isEmpty_bst(tree) == (tree->data.size == 0));
}

static void test_initialization(void) {
  bst tree = bst_init(compare_ints);

  CHECK(tree.data.root == NULL);
  CHECK(tree.data.size == 0);
  CHECK(tree.data.compare == compare_ints);
  CHECK(isEmpty_bst(&tree));
  check_bst_invariants(&tree);

  bst_clear(&tree);
}

static void test_insert_and_search(void) {
  int values[] = {8, 3, 10, 1, 6, 14, 4, 7, 13};
  int equivalent_six = 6;
  int missing = 99;
  bst tree = bst_init(compare_ints);

  for (size_t i = 0; i < 9; ++i) {
    bst_insert(&tree, &values[i]);
    CHECK(tree.data.size == i + 1);
    CHECK(bst_search(&tree, &values[i]));
    check_bst_invariants(&tree);
  }
  CHECK(bst_search(&tree, &equivalent_six));
  CHECK(!bst_search(&tree, &missing));

  bst_clear(&tree);
}

static void test_duplicate_insert_is_ignored(void) {
  int original = 5;
  int equivalent = 5;
  bst tree = bst_init(compare_ints);

  bst_insert(&tree, &original);
  b_pos original_root = tree.data.root;
  bst_insert(&tree, &equivalent);
  CHECK(tree.data.size == 1);
  CHECK(tree.data.root == original_root);
  CHECK(tree.data.root->value == &original);
  check_bst_invariants(&tree);

  bst_clear(&tree);
}

static void test_remove_missing(void) {
  int values[] = {5, 3, 7};
  int missing = 9;
  bst tree = bst_init(compare_ints);
  for (size_t i = 0; i < 3; ++i)
    bst_insert(&tree, &values[i]);

  bst_remove(&tree, &missing);
  CHECK(tree.data.size == 3);
  check_bst_invariants(&tree);
  bst_clear(&tree);
}

static void test_remove_leaf(void) {
  int values[] = {5, 3, 7, 2};
  bst tree = bst_init(compare_ints);
  for (size_t i = 0; i < 4; ++i)
    bst_insert(&tree, &values[i]);

  bst_remove(&tree, &values[3]);
  CHECK(!bst_search(&tree, &values[3]));
  CHECK(tree.data.size == 3);
  check_bst_invariants(&tree);
  bst_clear(&tree);
}

static void test_remove_node_with_left_child(void) {
  int values[] = {8, 5, 3};
  bst tree = bst_init(compare_ints);
  for (size_t i = 0; i < 3; ++i)
    bst_insert(&tree, &values[i]);

  bst_remove(&tree, &values[1]);
  CHECK(tree.data.root->left->value == &values[2]);
  CHECK(tree.data.root->left->father == tree.data.root);
  CHECK(!bst_search(&tree, &values[1]));
  check_bst_invariants(&tree);
  bst_clear(&tree);
}

static void test_remove_node_with_right_child(void) {
  int values[] = {5, 8, 10};
  bst tree = bst_init(compare_ints);
  for (size_t i = 0; i < 3; ++i)
    bst_insert(&tree, &values[i]);

  bst_remove(&tree, &values[1]);
  CHECK(tree.data.root->right->value == &values[2]);
  CHECK(tree.data.root->right->father == tree.data.root);
  CHECK(!bst_search(&tree, &values[1]));
  check_bst_invariants(&tree);
  bst_clear(&tree);
}

static void test_remove_root_cases(void) {
  int only = 1;
  bst singleton = bst_init(compare_ints);
  bst_insert(&singleton, &only);
  bst_remove(&singleton, &only);
  check_bst_invariants(&singleton);
  CHECK(isEmpty_bst(&singleton));
  bst_clear(&singleton);

  int values[] = {5, 3};
  bst one_child = bst_init(compare_ints);
  bst_insert(&one_child, &values[0]);
  bst_insert(&one_child, &values[1]);
  bst_remove(&one_child, &values[0]);
  CHECK(one_child.data.root->value == &values[1]);
  CHECK(one_child.data.root->father == NULL);
  check_bst_invariants(&one_child);
  bst_clear(&one_child);
}

static void test_remove_two_children_direct_successor(void) {
  int values[] = {5, 3, 7, 8};
  bst tree = bst_init(compare_ints);
  for (size_t i = 0; i < 4; ++i)
    bst_insert(&tree, &values[i]);

  bst_remove(&tree, &values[0]);
  CHECK(*(int *)tree.data.root->value == 7);
  CHECK(!bst_search(&tree, &values[0]));
  CHECK(bst_search(&tree, &values[3]));
  CHECK(tree.data.size == 3);
  check_bst_invariants(&tree);
  bst_clear(&tree);
}

static void test_remove_two_children_deep_successor(void) {
  int values[] = {20, 10, 30, 25, 40, 22, 24};
  bst tree = bst_init(compare_ints);
  for (size_t i = 0; i < 7; ++i)
    bst_insert(&tree, &values[i]);

  bst_remove(&tree, &values[0]);
  CHECK(*(int *)tree.data.root->value == 22);
  CHECK(!bst_search(&tree, &values[0]));
  CHECK(bst_search(&tree, &values[6]));
  CHECK(tree.data.size == 6);
  check_bst_invariants(&tree);
  bst_clear(&tree);
}

static void test_remove_every_value(void) {
  int values[] = {8, 3, 10, 1, 6, 14, 4, 7, 13};
  size_t removal_order[] = {0, 4, 3, 8, 1, 5, 2, 6, 7};
  bst tree = bst_init(compare_ints);
  for (size_t i = 0; i < 9; ++i)
    bst_insert(&tree, &values[i]);

  for (size_t i = 0; i < 9; ++i) {
    size_t index = removal_order[i];
    bst_remove(&tree, &values[index]);
    CHECK(!bst_search(&tree, &values[index]));
    CHECK(tree.data.size == 8 - i);
    check_bst_invariants(&tree);
  }
  CHECK(isEmpty_bst(&tree));
  bst_clear(&tree);
}

static void test_descending_comparator(void) {
  int values[] = {5, 2, 8, 1, 3, 7, 9};
  bst tree = bst_init(compare_ints_descending);
  for (size_t i = 0; i < 7; ++i) {
    bst_insert(&tree, &values[i]);
    check_bst_invariants(&tree);
  }
  CHECK(*(int *)tree.data.root->left->value == 8);
  CHECK(*(int *)tree.data.root->right->value == 2);

  bst_remove(&tree, &values[0]);
  check_bst_invariants(&tree);
  bst_clear(&tree);
}

static void test_clear_and_ownership(void) {
  int values[] = {2, 1, 3};
  bst tree = bst_init(compare_ints);
  for (size_t i = 0; i < 3; ++i)
    bst_insert(&tree, &values[i]);

  bst_clear(&tree);
  CHECK(tree.data.root == NULL);
  CHECK(tree.data.size == 0);
  CHECK(tree.data.compare == NULL);
  CHECK(values[0] == 2);
  CHECK(values[1] == 1);
  CHECK(values[2] == 3);

  bst_clear(&tree);
}

static void dies_init_without_comparator(void) {
  (void)bst_init(NULL);
}

static void test_invalid_initialization(void) {
  check_exits_with_failure(dies_init_without_comparator,
                           "BST without comparator");
}

int main(void) {
  test_initialization();
  test_insert_and_search();
  test_duplicate_insert_is_ignored();
  test_remove_missing();
  test_remove_leaf();
  test_remove_node_with_left_child();
  test_remove_node_with_right_child();
  test_remove_root_cases();
  test_remove_two_children_direct_successor();
  test_remove_two_children_deep_successor();
  test_remove_every_value();
  test_descending_comparator();
  test_clear_and_ownership();
  test_invalid_initialization();

  if (tests_failed != 0) {
    fprintf(stderr, "%u/%u tests failed\n", tests_failed, tests_run);
    return EXIT_FAILURE;
  }
  printf("All %u BST tests passed\n", tests_run);
  return EXIT_SUCCESS;
}
