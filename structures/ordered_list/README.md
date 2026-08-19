# Ordered list API

An `ordered_list` wraps the singly linked list and inserts every element in
comparator-defined ascending order. It owns its nodes, while callers retain
ownership of all stored element values.

## Quick start

```c
#include "ordered_list.h"

static int compare_ints(const elem left, const elem right) {
  const int a = *(const int *)left;
  const int b = *(const int *)right;
  return (a > b) - (a < b);
}

int main(void) {
  int a = 3, b = 1, c = 2, key = 2;
  ordered_list values = ordered_list_init(compare_ints);

  ordered_list_insert(&values, &a);
  ordered_list_insert(&values, &b);
  ordered_list_insert(&values, &c); /* Stored order: 1, 2, 3. */

  bool found = ordered_list_search(&values, &key);
  ordered_list_clear(&values);
  return found ? 0 : 1;
}
```

## Contracts and ownership

- The comparator passed to `ordered_list_init()` must be non-`NULL` and remain
  valid for the lifetime of the ordered list.
- The comparator defines both ordering and equality.
- Clear, remove, and deduplicate free nodes only. Stored values remain owned by
  the caller.
- Non-`NULL` positions must belong to the relevant list and become invalid when
  their nodes are removed.
- Directly modifying `ol.data` can break sorted order. Use this API instead.
- Fusion requires both inputs to use the exact same comparator function. It
  creates new nodes that share element pointers with the input lists.

Invalid positions and incompatible fusion comparators terminate the process via
the underlying list API or `errx()`.

## Function reference

| Function | Behavior | Complexity |
|---|---|---:|
| `ordered_list_init()` | Creates an empty list and stores its comparator | O(1) |
| `ordered_list_clear()` | Frees nodes, retains the comparator, and resets the list | O(n) |
| `isEmpty_ordered_list()` | Tests whether the list is empty | O(1) |
| `ordered_list_first()` | Returns the first position or `NULL` | O(1) |
| `ordered_list_end()` | Tests whether a position is `NULL` | O(1) |
| `ordered_list_next()` | Returns the following position | O(1) |
| `ordered_list_previous()` | Finds the preceding position | O(n) |
| `ordered_list_insert()` | Inserts at the comparator-defined location | O(n) |
| `ordered_list_remove()` | Removes a node, not its value | O(n) |
| `ordered_list_read()` | Returns the stored pointer | O(1) |
| `ordered_list_deduplicate()` | Removes adjacent comparator-equivalent values | O(n) |
| `ordered_list_fusion()` | Returns a new ordered list containing both inputs | O((n+m)²) currently |
| `ordered_list_search()` | Searches using comparator equality | O(n) |

There is intentionally no write operation: replacing a value in place could
violate sorted order. Remove the old node and insert the new value instead.

## Build and test

```sh
make test
make sanitize
make clean
```

The suite covers every public function, clear and reuse, ordering across input
patterns, duplicates, traversal, removal, fusion, search, invalid positions,
comparator compatibility, and empty-list behavior.
