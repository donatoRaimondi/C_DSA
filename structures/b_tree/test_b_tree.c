#include "b_tree.h"

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

static void check_list_values(const list *l, const int expected[],
                              size_t count) {
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

static void test_empty_tree(void) {
  b_tree bt = b_tree_init(compare_ints);

  CHECK(isEmpty_b_tree(&bt));
  CHECK(bt.size == 0);
  CHECK(bt.root == NULL);
  CHECK(bt.compare == compare_ints);
  CHECK(b_tree_root(&bt) == NULL);

  list recursive = b_tree_dfs_recursive(&bt);
  list iterative = b_tree_dfs(&bt);
  list breadth_first = b_tree_bfs(&bt);
  CHECK(isEmpty_list(&recursive));
  CHECK(isEmpty_list(&iterative));
  CHECK(isEmpty_list(&breadth_first));
  CHECK(!b_tree_search(&bt, NULL));

  list_clear(&recursive);
  list_clear(&iterative);
  list_clear(&breadth_first);
  b_tree_clear(&bt);
}

static void test_root_insertion(void) {
  int first = 1;
  int second = 2;
  b_tree bt = b_tree_init(compare_ints);

  b_tree_ins_root(&bt, &first);
  b_pos old_root = b_tree_root(&bt);
  CHECK(bt.size == 1);
  CHECK(b_tree_node_read(old_root) == &first);
  CHECK(b_tree_father(old_root) == NULL);
  CHECK(b_tree_sx_empty(old_root));
  CHECK(b_tree_dx_empty(old_root));

  b_tree_ins_root(&bt, &second);
  b_pos new_root = b_tree_root(&bt);
  CHECK(bt.size == 2);
  CHECK(b_tree_node_read(new_root) == &second);
  CHECK(b_tree_sx(new_root) == old_root);
  CHECK(b_tree_dx(new_root) == NULL);
  CHECK(b_tree_father(old_root) == new_root);
  CHECK(b_tree_father(new_root) == NULL);

  b_tree_clear(&bt);
}

static void test_left_and_right_insertion(void) {
  int values[] = {1, 2, 3};
  b_tree bt = b_tree_init(compare_ints);
  b_tree_ins_root(&bt, &values[0]);
  b_pos root = b_tree_root(&bt);

  b_tree_ins_sx(&bt, root, &values[1]);
  b_tree_ins_dx(&bt, root, &values[2]);
  b_pos left = b_tree_sx(root);
  b_pos right = b_tree_dx(root);
  CHECK(bt.size == 3);
  CHECK(b_tree_node_read(left) == &values[1]);
  CHECK(b_tree_node_read(right) == &values[2]);
  CHECK(b_tree_father(left) == root);
  CHECK(b_tree_father(right) == root);
  CHECK(b_tree_sx_empty(left));
  CHECK(b_tree_dx_empty(right));

  b_tree_clear(&bt);
}

static void test_insertion_into_occupied_children(void) {
  int values[] = {10, 20, 30, 15, 25};
  b_tree bt = b_tree_init(compare_ints);
  b_tree_ins_root(&bt, &values[0]);
  b_pos root = b_tree_root(&bt);
  b_tree_ins_sx(&bt, root, &values[1]);
  b_tree_ins_dx(&bt, root, &values[2]);
  b_pos old_left = b_tree_sx(root);
  b_pos old_right = b_tree_dx(root);

  b_tree_ins_sx(&bt, root, &values[3]);
  b_tree_ins_dx(&bt, root, &values[4]);
  b_pos new_left = b_tree_sx(root);
  b_pos new_right = b_tree_dx(root);
  CHECK(bt.size == 5);
  CHECK(b_tree_node_read(new_left) == &values[3]);
  CHECK(b_tree_sx(new_left) == old_left);
  CHECK(b_tree_father(old_left) == new_left);
  CHECK(b_tree_node_read(new_right) == &values[4]);
  CHECK(b_tree_dx(new_right) == old_right);
  CHECK(b_tree_father(old_right) == new_right);

  b_tree_clear(&bt);
}

static void test_null_father_inserts_root(void) {
  int first = 1;
  int second = 2;
  b_tree left_tree = b_tree_init(compare_ints);
  b_tree right_tree = b_tree_init(compare_ints);

  b_tree_ins_sx(&left_tree, NULL, &first);
  b_tree_ins_dx(&right_tree, NULL, &second);
  CHECK(left_tree.size == 1);
  CHECK(right_tree.size == 1);
  CHECK(b_tree_node_read(left_tree.root) == &first);
  CHECK(b_tree_node_read(right_tree.root) == &second);

  b_tree_clear(&left_tree);
  b_tree_clear(&right_tree);
}

static void test_read_and_write(void) {
  int old_value = 1;
  int new_value = 9;
  b_tree bt = b_tree_init(compare_ints);
  b_tree_ins_root(&bt, &old_value);

  b_tree_node_write(bt.root, &new_value);
  CHECK(b_tree_node_read(bt.root) == &new_value);
  CHECK(bt.size == 1);

  b_tree_clear(&bt);
}

static void build_complete_tree(b_tree *bt, int values[]) {
  b_tree_ins_root(bt, &values[0]);
  b_pos root = bt->root;
  b_tree_ins_sx(bt, root, &values[1]);
  b_tree_ins_dx(bt, root, &values[2]);
  b_tree_ins_sx(bt, root->left, &values[3]);
  b_tree_ins_dx(bt, root->left, &values[4]);
  b_tree_ins_sx(bt, root->right, &values[5]);
  b_tree_ins_dx(bt, root->right, &values[6]);
}

static void test_traversals(void) {
  int values[] = {1, 2, 3, 4, 5, 6, 7};
  const int preorder[] = {1, 2, 4, 5, 3, 6, 7};
  const int level_order[] = {1, 2, 3, 4, 5, 6, 7};
  b_tree bt = b_tree_init(compare_ints);
  build_complete_tree(&bt, values);

  list recursive = b_tree_dfs_recursive(&bt);
  list iterative = b_tree_dfs(&bt);
  list breadth_first = b_tree_bfs(&bt);
  check_list_values(&recursive, preorder, 7);
  check_list_values(&iterative, preorder, 7);
  check_list_values(&breadth_first, level_order, 7);
  CHECK(bt.size == 7);

  list_clear(&recursive);
  list_clear(&iterative);
  list_clear(&breadth_first);
  b_tree_clear(&bt);
}

static void test_search(void) {
  int values[] = {1, 2, 3, 4, 5, 6, 7};
  int root_key = 1;
  int leaf_key = 7;
  int missing = 99;
  b_tree bt = b_tree_init(compare_ints);
  build_complete_tree(&bt, values);

  CHECK(b_tree_search(&bt, &root_key));
  CHECK(b_tree_search(&bt, &leaf_key));
  CHECK(!b_tree_search(&bt, &missing));
  CHECK(bt.size == 7);

  b_tree_clear(&bt);
}

static void test_clear_and_ownership(void) {
  int values[] = {1, 2, 3};
  b_tree bt = b_tree_init(compare_ints);

  b_tree_clear(&bt);
  CHECK(bt.root == NULL);
  CHECK(bt.size == 0);
  CHECK(bt.compare == NULL);

  bt = b_tree_init(compare_ints);
  b_tree_ins_root(&bt, &values[0]);
  b_tree_ins_sx(&bt, bt.root, &values[1]);
  b_tree_ins_dx(&bt, bt.root, &values[2]);
  b_tree_clear(&bt);
  CHECK(bt.root == NULL);
  CHECK(bt.size == 0);
  CHECK(bt.compare == NULL);
  CHECK(values[0] == 1);
  CHECK(values[1] == 2);
  CHECK(values[2] == 3);

  b_tree_clear(&bt);
}

static void dies_init_without_comparator(void) {
  (void)b_tree_init(NULL);
}

static void dies_read_null_node(void) {
  (void)b_tree_node_read(NULL);
}

static void dies_write_null_node(void) {
  b_tree_node_write(NULL, NULL);
}

static void test_invalid_operations(void) {
  check_exits_with_failure(dies_init_without_comparator,
                           "tree without comparator");
  check_exits_with_failure(dies_read_null_node, "read NULL tree node");
  check_exits_with_failure(dies_write_null_node, "write NULL tree node");
}

int main(void) {
  test_empty_tree();
  test_root_insertion();
  test_left_and_right_insertion();
  test_insertion_into_occupied_children();
  test_null_father_inserts_root();
  test_read_and_write();
  test_traversals();
  test_search();
  test_clear_and_ownership();
  test_invalid_operations();

  if (tests_failed != 0) {
    fprintf(stderr, "%u/%u tests failed\n", tests_failed, tests_run);
    return EXIT_FAILURE;
  }
  printf("All %u binary-tree tests passed\n", tests_run);
  return EXIT_SUCCESS;
}
