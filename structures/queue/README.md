# Queue API

This module implements a first-in, first-out (FIFO) queue on top of the singly
linked list. It stores both the list head and a tail pointer, making push, pop,
and top constant-time operations.

## Quick start

```c
#include "queue.h"

int main(void) {
  int first = 10;
  int second = 20;
  queue values = queue_init();

  queue_push(&values, &first);
  queue_push(&values, &second);

  int *front = queue_top(&values);   /* 10; queue is unchanged. */
  int *popped = queue_pop(&values);  /* 10; value is removed. */

  queue_clear(&values);
  return front == popped ? 0 : 1;
}
```

## Contracts and ownership

- The queue stores `elem` pointers inherited from the list API.
- The caller owns the pointed-to values and must keep them alive while stored.
- `queue_pop()` and `queue_clear()` free nodes, not element values.
- `NULL` is a valid stored element. Therefore, a `NULL` result from
  `queue_pop()` or `queue_top()` can be a real value; call `isEmpty_queue()`
  before either operation.
- Calling `queue_pop()` or `queue_top()` on an empty queue terminates the
  process with `errx()`.
- When the queue is empty, both `data.head` and `tail` are `NULL`. When it is
  nonempty, `tail` identifies the final node and `tail->next` is `NULL`.
- Direct modification of `data` or `tail` can break these invariants. Prefer
  the queue API.

## Function reference

| Function | Behavior | Complexity |
|---|---|---:|
| `queue_init()` | Returns an empty queue | O(1) |
| `queue_clear()` | Frees all nodes and resets head/tail; values survive | O(n) |
| `isEmpty_queue()` | Tests whether the queue is empty | O(1) |
| `queue_push()` | Adds a value at the back | O(1) |
| `queue_pop()` | Removes and returns the front value | O(1) |
| `queue_top()` | Returns the front value without removing it | O(1) |

## Build and test

From this directory:

```sh
make test
make sanitize
make clean
```

The suite covers initialization, FIFO order, head/tail invariants, top without
removal, interleaved operations, `NULL` elements, repeated single-element
cycles, clear and reuse, element ownership, and empty pop/top failures.
