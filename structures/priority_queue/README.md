# Priority queue API

This module implements a dynamically growing binary-heap priority queue. The
value considered smallest by the comparator has the highest priority and is
stored at the heap root.

## Quick start

```c
#include "priority_queue.h"

static int compare_ints(const elem left, const elem right) {
  const int a = *(const int *)left;
  const int b = *(const int *)right;
  return (a > b) - (a < b);
}

int main(void) {
  int low = 10;
  int high = 1;
  priority_queue pq = priority_queue_init(2, compare_ints);

  priority_queue_push(&pq, &low);
  priority_queue_push(&pq, &high);

  int *first = priority_queue_pop(&pq); /* 1 */
  priority_queue_clear(&pq);
  return first == &high ? 0 : 1;
}
```

## Comparator and priority

The comparator follows the usual C convention:

- negative when `left` should come before `right`;
- zero when the values have equal priority;
- positive when `left` should come after `right`.

The queue is a min-heap with respect to that comparator. To create a max-priority
queue, reverse the comparison:

```c
static int compare_ints_descending(const elem left, const elem right) {
  const int a = *(const int *)left;
  const int b = *(const int *)right;
  return (b > a) - (b < a);
}
```

Elements with equal priority are supported, but their removal order is not
stable: insertion order among ties is not guaranteed.

## Contracts and ownership

- Initial capacity must be greater than zero and the comparator must be
  non-`NULL`.
- When full, the backing array doubles automatically. The public `capacity`
  field therefore may change after `priority_queue_push()`.
- The queue owns its backing array but never owns or frees stored `elem` values.
  Callers must keep those objects alive while they remain queued.
- `priority_queue_clear()` releases the array and resets every field. Assign a
  new result from `priority_queue_init()` before reusing the cleared object.
- `priority_queue_pop()` and `priority_queue_peek()` return `NULL` on an empty
  queue.
- `NULL` can also be stored if the comparator supports it. In that case, use
  `isEmpty_priority_queue()` to distinguish an empty result from a stored
  `NULL` value.
- Directly modifying `data`, `size`, or `capacity` can break the heap invariant
  and must be avoided.

The heap invariant is:

```text
compare(parent, child) <= 0
```

for every occupied child position.

## Function reference

| Function | Behavior | Complexity |
|---|---|---:|
| `priority_queue_init()` | Allocates an empty heap | O(capacity) |
| `priority_queue_clear()` | Frees the array and resets all fields | O(1) |
| `isEmpty_priority_queue()` | Tests whether `size` is zero | O(1) |
| `priority_queue_push()` | Inserts and restores the heap upward | O(log n) amortized |
| `priority_queue_pop()` | Removes the root and restores the heap downward | O(log n) |
| `priority_queue_peek()` | Returns the root without removal | O(1) |

A push that triggers reallocation additionally copies O(n) pointers, so that
individual operation is O(n); across repeated insertions, growth is amortized.

## Build and test

From this directory:

```sh
make test
make sanitize
make clean
```

The suite covers initialization, empty behavior, heap invariants, duplicates,
negative values, sorted pop order, automatic growth through several capacity
doublings, custom reversed priority, interleaved operations, supported `NULL`
values, clear and ownership, and invalid initialization.

Failure tests use `fork()` and `waitpid()`, so the suite targets macOS and Linux
rather than native Windows.
