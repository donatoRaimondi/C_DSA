# Binary tree API

This module implements a generic, linked binary tree with parent pointers. It
is a positional binary tree, not a binary search tree: callers explicitly choose
whether to insert on the left (`sx`) or right (`dx`).

## Quick start

```c
#include "b_tree.h"

static int compare_ints(const elem left, const elem right) {
  const int a = *(const int *)left;
  const int b = *(const int *)right;
  return (a > b) - (a < b);
}

int main(void) {
  int root_value = 1;
  int left_value = 2;
  int right_value = 3;
  int key = 3;
  b_tree tree = b_tree_init(compare_ints);

  b_tree_ins_root(&tree, &root_value);
  b_tree_ins_sx(&tree, b_tree_root(&tree), &left_value);
  b_tree_ins_dx(&tree, b_tree_root(&tree), &right_value);

  bool found = b_tree_search(&tree, &key);
  b_tree_clear(&tree);
  return found ? 0 : 1;
}
```

`sx` means left (*sinistra*) and `dx` means right (*destra*).

## Insertion semantics

Insertion never discards an existing subtree:

- `b_tree_ins_root()` creates a new root. The previous root becomes its left
  child.
- `b_tree_ins_sx()` inserts a new left child. If a left child already exists,
  that old child becomes the new node's left child.
- `b_tree_ins_dx()` behaves symmetrically on the right.
- Passing `NULL` as the father to either child-insertion function is equivalent
  to inserting a root.

For example, inserting `B` to the left of `A`, followed by `C` to the same
position, produces:

```text
    A
   /
  C
 /
B
```

Every insertion updates the affected `father` pointers and increments `size`.

## Contracts and ownership

- The comparator supplied to `b_tree_init()` must be non-`NULL` and remain
  valid while the tree is used. It defines equality for `b_tree_search()`.
- The tree owns and frees its nodes, but never frees stored `elem` values. Those
  objects remain caller-owned and must outlive their nodes.
- A non-`NULL` father passed to insertion must belong to that tree. Membership
  is a caller precondition and is not checked.
- Positions become invalid after `b_tree_clear()`.
- `b_tree_clear()` resets `root`, `size`, and `compare`. Assign a new result
  from `b_tree_init()` before reusing the cleared object.
- Node accessors require a non-`NULL` node.
- Public node fields permit manual inspection, but modifying links directly can
  break parent links, size accounting, traversal, or clearing.

## Traversals

All traversal functions return a new `list` whose nodes contain the same value
pointers as the tree. The caller owns that result and must call `list_clear()`.
Clearing the traversal list does not modify the tree or its values.

- `b_tree_dfs_recursive()` returns recursive preorder: node, left, right.
- `b_tree_dfs()` returns the same preorder using an explicit stack.
- `b_tree_bfs()` returns level order using a queue.

An empty tree produces an empty list.

## Function reference

| Function | Behavior | Complexity |
|---|---|---:|
| `b_tree_init()` | Creates an empty tree with a comparator | O(1) |
| `isEmpty_b_tree()` | Tests whether `size` is zero | O(1) |
| `b_tree_clear()` | Frees all nodes and resets the tree | O(n) |
| `b_tree_root()` | Returns the root or `NULL` | O(1) |
| `b_tree_father()` | Returns a node's parent | O(1) |
| `b_tree_sx_empty()` / `b_tree_dx_empty()` | Tests for a missing child | O(1) |
| `b_tree_sx()` / `b_tree_dx()` | Returns a child | O(1) |
| `b_tree_ins_root()` | Inserts above the current root | O(1) |
| `b_tree_ins_sx()` / `b_tree_ins_dx()` | Inserts at one child position | O(1) |
| `b_tree_node_read()` / `b_tree_node_write()` | Reads or replaces a value | O(1) |
| `b_tree_dfs_recursive()` | Builds a recursive preorder list | O(n²) currently |
| `b_tree_dfs()` | Builds an iterative preorder list | O(n²) currently |
| `b_tree_bfs()` | Builds a level-order list | O(n²) currently |
| `b_tree_search()` | Searches with comparator equality | O(n) |

Traversal is currently O(n²) because appending each result to the singly linked
list scans to its tail. Traversal of the tree itself is O(n); a constant-time
list append operation would make the complete traversal O(n).

Recursive DFS additionally uses O(h) call-stack space, where `h` is tree
height. Iterative DFS and BFS use O(n) auxiliary nodes in the worst case.

## Build and test

From this directory:

```sh
make test
make sanitize
make clean
```

The suite covers empty trees, root/left/right insertion, occupied-child
insertion, parent links, reads and writes, recursive and iterative preorder,
level order, search, clear and ownership, size consistency, missing comparators,
and invalid node access.

Failure tests use `fork()` and `waitpid()`, so the suite targets macOS and Linux
rather than native Windows.
