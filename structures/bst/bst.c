#include "bst.h"
#include <stdlib.h>

bst bst_init(tree_compare_fn compare) {
  bst tree = {.data = b_tree_init(compare)};

  return tree;
}

bool isEmpty_bst(const bst *tree) { return isEmpty_b_tree(&tree->data); }

void bst_clear(bst *tree) { b_tree_clear(&tree->data); }

void bst_insert(bst *tree, elem value) {
  if (isEmpty_bst(tree)) {
    b_tree_ins_root(&tree->data, value);
    return;
  }

  b_pos curr = b_tree_root(&tree->data);

  while (curr != NULL) {
    int cmp = tree->data.compare(value, curr->value);

    if (cmp == 0)
      return;

    if (cmp < 0) {
      if (b_tree_sx_empty(curr)) {
        b_tree_ins_sx(&tree->data, curr, value);
        return;
      }

      curr = b_tree_sx(curr);
    } else {
      if (b_tree_dx_empty(curr)) {
        b_tree_ins_dx(&tree->data, curr, value);
        return;
      }

      curr = b_tree_dx(curr);
    }
  }
}

static b_pos bst_find_node(const bst *tree, elem value) {
  b_pos curr = b_tree_root(&tree->data);

  while (curr != NULL) {
    int cmp = tree->data.compare(value, curr->value);

    if (cmp == 0)
      return curr;

    curr = cmp < 0 ? curr->left : curr->right;
  }

  return NULL;
}

static b_pos bst_min_node(b_pos node) {
  while (node->left != NULL)
    node = node->left;

  return node;
}

void bst_remove(bst *tree, elem value) {
  b_pos node = bst_find_node(tree, value);

  if (node == NULL)
    return;

  /*
   * Two children:
   * replace the value with the inorder successor's value,
   * then physically remove the successor instead.
   */
  if (node->left != NULL && node->right != NULL) {
    b_pos successor = bst_min_node(node->right);

    node->value = successor->value;
    node = successor;
  }

  /*
   * At this point node has at most one child.
   */
  b_pos child = node->left != NULL ? node->left : node->right;

  /*
   * Removing the root.
   */
  if (node->father == NULL) {
    tree->data.root = child;

    if (child != NULL)
      child->father = NULL;
  }

  /*
   * node is the left child of its father.
   */
  else if (node->father->left == node) {
    node->father->left = child;

    if (child != NULL)
      child->father = node->father;
  }

  /*
   * node is the right child of its father.
   */
  else {
    node->father->right = child;

    if (child != NULL)
      child->father = node->father;
  }

  free(node);
  tree->data.size--;
}

bool bst_search(const bst *tree, elem value) {
  b_pos curr = b_tree_root(&tree->data);

  while (curr != NULL) {
    int cmp = tree->data.compare(value, curr->value);

    if (cmp == 0)
      return true;

    curr = cmp < 0 ? curr->left : curr->right;
  }

  return false;
}
