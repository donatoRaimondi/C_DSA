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

/* Create an empty tree. compare must be non-NULL and remain valid. O(1). */
b_tree b_tree_init(tree_compare_fn compare);

/* Return whether the tree contains no nodes. O(1). */
bool isEmpty_b_tree(const b_tree *bt);

/* Free all nodes and reset the tree; stored values are not freed. O(n). */
void b_tree_clear(b_tree *bt);

/* Return the root, or NULL when empty. O(1). */
b_pos b_tree_root(const b_tree *bt);
/* Return a non-NULL node's parent, or NULL for the root. O(1). */
b_pos b_tree_father(b_pos node);

/* Return whether a non-NULL node has no left child. O(1). */
bool b_tree_sx_empty(b_pos node);
/* Return whether a non-NULL node has no right child. O(1). */
bool b_tree_dx_empty(b_pos node);

/* Return a non-NULL node's left child. O(1). */
b_pos b_tree_sx(b_pos father);
/* Return a non-NULL node's right child. O(1). */
b_pos b_tree_dx(b_pos father);

/* Insert a new root, moving the previous root below it on the left. O(1). */
void b_tree_ins_root(b_tree *bt, elem value);
/* Insert as left child; an existing left subtree moves below the new node. */
void b_tree_ins_sx(b_tree *bt, b_pos father, elem value);
/* Insert as right child; an existing right subtree moves below the new node. */
void b_tree_ins_dx(b_tree *bt, b_pos father, elem value);

/* Return the value stored by a valid node. O(1). */
elem b_tree_node_read(b_pos node);
/* Replace the value stored by a valid node. O(1). */
void b_tree_node_write(b_pos node, elem value);

/* Return a new list containing recursive preorder DFS values. */
list b_tree_dfs_recursive(const b_tree *bt);
/* Return a new list containing iterative preorder DFS values. */
list b_tree_dfs(const b_tree *bt);

/* Return a new list containing breadth-first/level-order values. */
list b_tree_bfs(const b_tree *bt);

/* Search using comparator equality. O(n). */
bool b_tree_search(const b_tree *bt, elem value);

#endif
