# Dictionary API

This module implements a fixed-capacity generic dictionary (hash map) using
open addressing, linear probing, and tombstones for deleted entries.

## Quick start

```c
#include "dictionary.h"

static size_t hash_int(const elem key) {
  return (size_t)*(const int *)key;
}

static bool equal_int(const elem left, const elem right) {
  return *(const int *)left == *(const int *)right;
}

int main(void) {
  int key = 7;
  int value = 70;
  int equivalent_key = 7;
  dictionary d = dictionary_init(8, hash_int, equal_int);

  dictionary_insert(&d, &key, &value);
  int *found = dictionary_get(&d, &equivalent_key);

  dictionary_clear(&d);
  return found == &value ? 0 : 1;
}
```

## Contracts and ownership

- `capacity` must be greater than zero. The dictionary does not resize.
- `hash` and `equal` must be non-`NULL` and remain valid while the dictionary
  is used.
- If `equal(a, b)` is true, `hash(a)` and `hash(b)` must return the same hash.
- Keys and values are stored as pointers. The dictionary never copies or frees
  the pointed-to objects; the caller owns them and must keep them alive.
- Inserting an equivalent key replaces its value without increasing `size`.
  The originally stored key pointer remains in the slot.
- Removing a missing key has no effect. Removed slots become tombstones, which
  preserve probing chains and can be reused by later insertions.
- `NULL` is a valid value. Use `dictionary_contains()` to distinguish a stored
  `NULL` value from an absent key before calling `dictionary_get()`.
- Keys may be `NULL` only if both callbacks explicitly support `NULL`.
- Inserting a new key into a full table terminates the process with `errx()`.
- Getting a missing key or getting from an empty dictionary also terminates.
- `dictionary_clear()` frees the slot array and resets the callbacks and
  capacity. Assign a new result from `dictionary_init()` before reuse.

## Function reference

| Function | Behavior | Expected / worst complexity |
|---|---|---:|
| `dictionary_init()` | Allocates an empty fixed-capacity table | O(capacity) |
| `dictionary_clear()` | Frees the table and resets all fields | O(1) |
| `isEmpty_dictionary()` | Tests whether `size` is zero | O(1) |
| `dictionary_contains()` | Tests whether a key exists | O(1) / O(capacity) |
| `dictionary_insert()` | Inserts a pair or updates an equivalent key | O(1) / O(capacity) |
| `dictionary_remove()` | Removes a key if present | O(1) / O(capacity) |
| `dictionary_get()` | Returns the value for an existing key | O(1) / O(capacity) |

Performance depends on the load factor and quality of the hash function.
Because the table is fixed-size, callers should choose a capacity comfortably
larger than the expected number of entries.

## Build and test

From this directory:

```sh
make test
make sanitize
make clean
```

The suite covers initialization, insertion, lookup, callback-based equivalent
keys, value replacement, collisions, complete table utilization, tombstones,
deletion and reuse, `NULL` values, clear and ownership, invalid callbacks, zero
capacity, missing keys, full-table insertion, and operations after clear.

Failure tests use `fork()` and `waitpid()`, so the suite targets macOS and Linux
rather than native Windows.
