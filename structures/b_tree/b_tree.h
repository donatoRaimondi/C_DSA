#ifndef B_TREE_H
#define B_TREE_H

#include "../list/list.h"
#include <stdbool.h>
#include <stddef.h>

typedef int (*tree_compare_fn)(const elem left, const elem right);

typedef struct b_tree_node {
  elem value;
  struct b_tree_node *father;
  struct b_tree_node *left;
  struct b_tree_node *right;
} b_tree_node;

typedef b_tree_node *b_pos;

typedef struct {
  b_pos root;
  size_t size;
  tree_compare_fn compare;
} b_tree;

b_tree b_tree_init(tree_compare_fn compare);

bool isEmpty_b_tree(const b_tree *bt);

void b_tree_clear(b_tree *bt);

b_pos b_tree_root(const b_tree *bt);
b_pos b_tree_father(b_pos node);

bool b_tree_sx_empty(b_pos node);
bool b_tree_dx_empty(b_pos node);

b_pos b_tree_sx(b_pos father);
b_pos b_tree_dx(b_pos father);

void b_tree_ins_root(b_tree *bt, elem value);
void b_tree_ins_sx(b_tree *bt, b_pos father, elem value);
void b_tree_ins_dx(b_tree *bt, b_pos father, elem value);

elem b_tree_node_read(b_pos node);
void b_tree_node_write(b_pos node, elem value);

#endif
