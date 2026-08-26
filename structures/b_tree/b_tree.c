#include "b_tree.h"
#include "../queue/queue.h"
#include "../stack/stack.h"
#include <err.h>
#include <stdlib.h>

b_tree b_tree_init(tree_compare_fn compare) {
  if (compare == NULL)
    errx(EXIT_FAILURE, "b_tree_init: compare must be non-NULL");

  b_tree bt = {.root = NULL, .size = 0, .compare = compare};

  return bt;
}

bool isEmpty_b_tree(const b_tree *bt) { return bt->size == 0; }

void b_tree_clear(b_tree *bt) {
  if (bt->root != NULL) {
    stack nodes = stack_init();
    stack_push(&nodes, bt->root);

    while (!isEmpty_stack(&nodes)) {
      b_pos current = stack_pop(&nodes);

      if (current->left != NULL)
        stack_push(&nodes, current->left);
      if (current->right != NULL)
        stack_push(&nodes, current->right);

      free(current);
    }

    stack_clear(&nodes);
  }

  bt->root = NULL;
  bt->size = 0;
  bt->compare = NULL;
}

b_pos b_tree_root(const b_tree *bt) { return bt->root; }

b_pos b_tree_father(b_pos node) { return node->father; }

bool b_tree_sx_empty(b_pos node) { return node->left == NULL; }

bool b_tree_dx_empty(b_pos node) { return node->right == NULL; }

b_pos b_tree_sx(b_pos father) { return father->left; }

b_pos b_tree_dx(b_pos father) { return father->right; }

void b_tree_ins_root(b_tree *bt, elem value) {
  b_pos new = malloc(sizeof *new);

  if (new == NULL)
    err(EXIT_FAILURE, "b_tree_ins_root: malloc failed");

  new->value = value;
  new->father = NULL;
  new->left = NULL;
  new->right = NULL;

  if (isEmpty_b_tree(bt)) {
    bt->root = new;
    bt->size++;
    return;
  }

  new->left = bt->root;
  bt->root->father = new;
  bt->root = new;
  bt->size++;
}

void b_tree_ins_sx(b_tree *bt, b_pos father, elem value) {
  // inserting a new root
  if (father == NULL) {
    b_tree_ins_root(bt, value);
    return;
  }

  b_pos new = malloc(sizeof *new);

  if (new == NULL)
    err(EXIT_FAILURE, "b_tree_ins_sx: malloc failed");

  new->value = value;
  new->father = NULL;
  new->left = NULL;
  new->right = NULL;

  // inserting at the end
  if (b_tree_sx_empty(father)) {
    father->left = new;
    new->father = father;
  } else {
    b_pos old_sx = father->left;
    father->left = new;
    new->father = father;

    new->left = old_sx;
    old_sx->father = new;
  }

  bt->size++;
  return;
}

void b_tree_ins_dx(b_tree *bt, b_pos father, elem value) {
  if (father == NULL) {
    b_tree_ins_root(bt, value);
    return;
  }

  b_pos new = malloc(sizeof *new);

  if (new == NULL)
    err(EXIT_FAILURE, "b_tree_ins_dx: malloc failed");

  new->value = value;
  new->father = NULL;
  new->left = NULL;
  new->right = NULL;

  if (b_tree_dx_empty(father)) {
    father->right = new;
    new->father = father;
  } else {
    b_pos old_dx = father->right;

    father->right = new;
    new->father = father;

    new->right = old_dx;
    old_dx->father = new;
  }

  bt->size++;
}

elem b_tree_node_read(b_pos node) {
  if (node == NULL)
    errx(EXIT_FAILURE, "b_tree_node_read: invalid node");

  return node->value;
}

void b_tree_node_write(b_pos node, elem value) {
  if (node == NULL)
    errx(EXIT_FAILURE, "b_tree_node_write: invalid node");
  node->value = value;
}

static void dfs_preorder_node(b_pos node, list *out) {
  if (node == NULL)
    return;

  list_insert(out, b_tree_node_read(node), NULL);

  dfs_preorder_node(b_tree_sx(node), out);
  dfs_preorder_node(b_tree_dx(node), out);
}

// pre-visita
list b_tree_dfs_recursive(const b_tree *bt) {
  list return_list = list_init();

  if (isEmpty_b_tree(bt))
    return return_list;

  dfs_preorder_node(b_tree_root(bt), &return_list);

  return return_list;
}

list b_tree_dfs(const b_tree *bt) {
  list return_list = list_init();

  if (isEmpty_b_tree(bt))
    return return_list;

  // simula lo stack di sistema
  stack s = stack_init();

  b_pos curr = b_tree_root(bt);
  stack_push(&s, curr);

  while (!isEmpty_stack(&s)) {
    curr = stack_pop(&s);

    list_insert(&return_list, curr->value, NULL);

    if (!b_tree_dx_empty(curr))
      stack_push(&s, b_tree_dx(curr));

    if (!b_tree_sx_empty(curr))
      stack_push(&s, b_tree_sx(curr));
  }

  stack_clear(&s);
  return return_list;
}

list b_tree_bfs(const b_tree *bt) {
  list return_list = list_init();

  if (isEmpty_b_tree(bt))
    return return_list;

  queue q = queue_init();
  queue_push(&q, b_tree_root(bt));

  while (!isEmpty_queue(&q)) {
    b_pos curr = queue_pop(&q);

    if (!b_tree_sx_empty(curr))
      queue_push(&q, b_tree_sx(curr));

    if (!b_tree_dx_empty(curr))
      queue_push(&q, b_tree_dx(curr));

    list_insert(&return_list, curr->value, NULL);
  }

  queue_clear(&q);
  return return_list;
}

bool b_tree_search(const b_tree *bt, elem value) {
  if (isEmpty_b_tree(bt))
    return false;

  stack s = stack_init();
  stack_push(&s, b_tree_root(bt));

  while (!isEmpty_stack(&s)) {
    b_pos curr = stack_pop(&s);

    if (bt->compare(b_tree_node_read(curr), value) == 0) {
      stack_clear(&s);
      return true;
    }

    if (!b_tree_dx_empty(curr))
      stack_push(&s, b_tree_dx(curr));

    if (!b_tree_sx_empty(curr))
      stack_push(&s, b_tree_sx(curr));
  }

  stack_clear(&s);
  return false;
}
