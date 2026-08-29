#ifndef BST_H
#define BST_H

#include "../b_tree/b_tree.h"

typedef struct {
  b_tree data;
} bst;

/* Create an empty BST. compare must be non-NULL and remain valid. O(1). */
bst bst_init(tree_compare_fn compare);

/* Return whether the tree contains no nodes. O(1). */
bool isEmpty_bst(const bst *tree);

/* Free every node and reset the tree; stored values are not freed. O(n). */
void bst_clear(bst *tree);

/* Insert value if no comparator-equivalent value exists. O(h). */
void bst_insert(bst *tree, elem value);

/* Remove a comparator-equivalent value if present. O(h). */
void bst_remove(bst *tree, elem value);

/* Return whether a comparator-equivalent value exists. O(h). */
bool bst_search(const bst *tree, elem value);

#endif // !BST_H
