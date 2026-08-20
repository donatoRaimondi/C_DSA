# Hash set API

This module implements a fixed-capacity generic set using open addressing,
linear probing, and tombstones for deleted slots.

## Quick start

```c
#include "set.h"

static size_t hash_int(const elem value) {
  return (size_t)*(const int *)value;
}

static bool equal_int(const elem left, const elem right) {
  return *(const int *)left == *(const int *)right;
}

int main(void) {
  int a = 10, b = 20, equivalent_a = 10;
  set values = set_init(8, hash_int, equal_int);

  set_insert(&values, &a);
  set_insert(&values, &b);

  bool found = set_contains(&values, &equivalent_a); /* true */
  set_remove(&values, &a);
  set_clear(&values);
  return found ? 0 : 1;
}
```

## Contracts and ownership

- `capacity` must be greater than zero. The table does not resize.
- `hash` and `equal` must be non-`NULL` and remain valid while the set is used.
- If `equal(a, b)` is true, `hash(a)` and `hash(b)` must produce the same hash.
- The set stores `elem` pointers without copying or freeing their values. The
  caller owns those values and must keep them alive while stored.
- Inserting an equivalent value is a no-op and does not increase `size`.
- Removing a missing value is a no-op. Deleted slots become tombstones and can
  be reused by later insertions.
- Inserting a new value into a full table terminates the process with `errx()`.
- `set_clear()` frees the slot array and resets callbacks and capacity. Assign a
  new result from `set_init()` before reusing the cleared object.
- `NULL` can only be used as a value if both callbacks explicitly support it.

Union, intersection, and difference require the two operands to use the exact
same hash and equality function pointers. Their results contain new slots but
share element pointers with their inputs; the caller must clear every result.

## Function reference

| Function | Behavior | Expected / worst complexity |
|---|---|---:|
| `set_init()` | Allocates an empty fixed-capacity table | O(capacity) |
| `set_clear()` | Frees the table and resets all fields | O(1) |
| `isEmpty_set()` | Tests whether `size` is zero | O(1) |
| `set_contains()` | Tests membership | O(1) / O(capacity) |
| `set_insert()` | Adds a value if it is absent | O(1) / O(capacity) |
| `set_remove()` | Removes a value if present | O(1) / O(capacity) |
| `set_union()` | Returns all values in A or B | O(n+m) expected |
| `set_intersect()` | Returns values present in both A and B | O(n) expected |
| `set_difference()` | Returns values in A but not B | O(n) expected |

Worst-case set-operation time can be quadratic when collisions force every
lookup or insertion to scan most of a table.

## Build and test

From this directory:

```sh
make test
make sanitize
make clean
```

The suite covers initialization, callback equality, duplicate insertion,
collisions, complete table utilization, tombstones, deletion and reuse, clear
and ownership, union/intersection/difference, empty operands, invalid callbacks,
zero capacity, full-table insertion, and incompatible operands.

Failure tests use `fork()` and `waitpid()`, so the suite targets macOS and Linux
rather than native Windows.
