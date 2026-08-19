# Singly linked list API

This module implements a generic singly linked list. It owns its nodes, but it
never owns, copies, or frees the objects referenced by `elem`.

## Quick start

```c
#include "list.h"

static int compare_ints(const elem left, const elem right) {
  const int a = *(const int *)left;
  const int b = *(const int *)right;
  return (a > b) - (a < b);
}

int main(void) {
  int a = 3, b = 1, c = 2, key = 2;
  list values = list_init();

  list_insert(&values, &a, NULL); /* NULL appends. */
  list_insert(&values, &b, NULL);
  list_insert(&values, &c, NULL);
  list_natural_mergesort(&values, compare_ints);

  bool found = list_search(&values, &key, compare_ints);
  list_clear(&values); /* Frees nodes, not a, b, c, or key. */
  return found ? 0 : 1;
}
```

## Contracts and ownership

- `elem` is `void *`. The caller owns the pointed-to object and must keep it
  alive while the list stores that pointer.
- `pos` identifies a node. `NULL` is the end position and means append when
  passed to `list_insert()`.
- A non-`NULL` position supplied to an operation must belong to that list.
- Removing a node or clearing a list invalidates positions to freed nodes.
- Sorting relinks nodes. Existing positions still identify their nodes, but
  their previous and next relationships may change.
- A comparator returns less than, equal to, or greater than zero when the left
  value sorts before, equals, or sorts after the right value. Comparators must
  be non-`NULL` and handle every stored value used with them.
- Storing `NULL` as an element is permitted, but any comparator applied to it
  must know how to handle `NULL`.

Invalid operations such as reading an empty list or advancing past the end call
`err()`/`errx()` and terminate the process. They are not recoverable errors.

## Function reference

| Function | Behavior | Complexity |
|---|---|---:|
| `list_init()` | Returns an empty list | O(1) |
| `list_clear()` | Frees all nodes and resets the list; values survive | O(n) |
| `isEmpty_list()` | Tests whether the list is empty | O(1) |
| `list_first()` | Returns the head or `NULL` | O(1) |
| `list_end()` | Tests whether a position is `NULL` | O(1) |
| `list_next()` | Returns the next position | O(1) |
| `list_previous()` | Finds the preceding position | O(n) |
| `list_insert()` | Inserts before a position; `NULL` appends | O(1) head, O(n) otherwise |
| `list_remove()` | Removes a node, not its value | O(1) head, O(n) otherwise |
| `list_read()` | Returns the stored pointer | O(1) |
| `list_write()` | Replaces the stored pointer | O(1) |
| `list_deduplicate()` | Keeps the first comparator-equivalent occurrence | O(n²) |
| `list_fusion()` | Copies two sorted lists into a new sorted list | O((n+m)²) currently |
| `list_natural_mergesort()` | Stably sorts by relinking natural runs | O(n) best, O(n log n) worst |
| `list_search()` | Searches using comparator equality | O(n) |

`list_fusion()` requires sorted inputs. It creates new nodes but stores the same
element pointers as its inputs. The caller must eventually clear the result.

## Build and test

```sh
make test
make sanitize
make clean
```

The suite covers every public function, traversal and mutation, clear and
reuse, comparator-based deduplication and search, fusion with empty inputs,
stable natural merge sort, invalid positions, and missing comparators.

Failure tests use `fork()` and `waitpid()`, so the suite targets macOS and Linux
rather than native Windows.
