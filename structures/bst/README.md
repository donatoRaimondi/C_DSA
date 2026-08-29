# Binary search tree API

This module implements a generic, unbalanced binary search tree (BST) on top of
the positional `b_tree` structure. Comparator-equivalent values are treated as
the same key, so the tree has set semantics and does not store duplicates.

It is not self-balancing: insertion order determines its shape.

## Quick start

```c
#include "bst.h"

static int compare_ints(const elem left, const elem right) {
  const int a = *(const int *)left;
  const int b = *(const int *)right;
  return (a > b) - (a < b);
}

int main(void) {
  int values[] = {8, 3, 10, 1, 6};
  int key = 6;
  bst tree = bst_init(compare_ints);

  for (size_t i = 0; i < 5; ++i)
    bst_insert(&tree, &values[i]);

  bool found = bst_search(&tree, &key);
  bst_remove(&tree, &key);
  bst_clear(&tree);
  return found ? 0 : 1;
}
```

## Ordering invariant

For every node:

```text
all values in left subtree  < node value
all values in right subtree > node value
```

Here `<` and `>` mean the ordering defined by the comparator, not necessarily
numeric ascending order. Reversing the comparator creates a tree whose physical
left/right arrangement is reversed while remaining a valid BST for that
comparator.

The comparator must define a consistent strict ordering:

- `compare(a, b) < 0` means `a` precedes `b`;
- `compare(a, b) == 0` means the keys are equivalent;
- `compare(a, b) > 0` means `a` follows `b`.

## Duplicate policy

`bst_insert()` ignores a value when a comparator-equivalent key already exists.
It does not increase `size` or replace the original stored pointer:

```c
int original = 5;
int equivalent = 5;

bst_insert(&tree, &original);
bst_insert(&tree, &equivalent); /* No change; original remains stored. */
```

## Removal cases

`bst_remove()` implements the three standard BST deletion cases:

1. A leaf is detached and freed.
2. A node with one child is replaced by that child; the child’s `father`
   pointer is updated.
3. A node with two children receives the value of its inorder successor—the
   minimum node in its right subtree—then that successor is physically removed.

Removing a missing value is a no-op. Removing the root correctly promotes its
child and resets the new root’s `father` to `NULL`.

## Contracts and ownership

- `bst_init()` requires a non-`NULL` comparator that remains valid while the
  tree is used.
- The BST owns and frees tree nodes, but never frees stored `elem` values. The
  caller owns those objects and must keep them alive while stored.
- Removal frees the matching tree node, not the caller’s value.
- `bst_clear()` frees all nodes and resets the underlying root, size, and
  comparator. Assign a new result from `bst_init()` before reuse.
- `tree.data` is public because the implementation wraps `b_tree`, but directly
  changing nodes or links can violate ordering, parent pointers, and size.
- Storing `NULL` requires a comparator that explicitly supports `NULL`.

## Function reference

Let `h` be the height of the tree.

| Function | Behavior | Complexity |
|---|---|---:|
| `bst_init()` | Creates an empty BST | O(1) |
| `isEmpty_bst()` | Tests whether the BST is empty | O(1) |
| `bst_clear()` | Frees all nodes and resets the tree | O(n) |
| `bst_insert()` | Inserts a unique key | O(h) |
| `bst_remove()` | Removes a key if present | O(h) |
| `bst_search()` | Searches for an equivalent key | O(h) |

For a reasonably balanced shape, `h = O(log n)`. For already sorted or
reverse-sorted insertion, this implementation can degenerate into a chain with
`h = O(n)`. No AVL or red-black rotations are performed.

Traversals are available through the wrapped binary tree, for example:

```c
list preorder = b_tree_dfs(&tree.data);
list_clear(&preorder);
```

## Build and test

From this directory:

```sh
make test
make sanitize
make clean
```

The suite validates the complete ordering invariant, every parent pointer,
reachable-node count, and stored `size` after mutations. It covers insertion,
duplicates, search, missing removal, leaf removal, one-child removal, all root
cases, direct and deep inorder successors, repeated removal to empty, reversed
comparators, clear and ownership, and invalid initialization.

Failure tests use `fork()` and `waitpid()`, so the suite targets macOS and Linux
rather than native Windows.
