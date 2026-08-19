# Stack API

This module implements a last-in, first-out (LIFO) stack on top of the singly
linked list. Push, pop, and top all operate at the list head and take constant
time.

## Quick start

```c
#include "stack.h"

int main(void) {
  int first = 10;
  int second = 20;
  stack values = stack_init();

  stack_push(&values, &first);
  stack_push(&values, &second);

  int *top = stack_top(&values); /* 20; stack is unchanged. */
  int *popped = stack_pop(&values); /* 20; value is removed. */

  stack_clear(&values);
  return top == popped ? 0 : 1;
}
```

## Contracts and ownership

- The stack stores `elem` pointers inherited from the list API.
- The caller owns the pointed-to values and must keep them alive while stored.
- `stack_pop()` and `stack_clear()` free nodes, not element values.
- `NULL` is a valid stored element. Consequently, a `NULL` result from
  `stack_pop()` or `stack_top()` can represent a stored value; use
  `isEmpty_stack()` before calling either operation.
- Calling `stack_pop()` or `stack_top()` on an empty stack terminates the
  process through the underlying list API.
- Direct modification of `stack.data` can violate stack abstraction. Prefer
  the stack functions.

## Function reference

| Function | Behavior | Complexity |
|---|---|---:|
| `stack_init()` | Returns an empty stack | O(1) |
| `stack_clear()` | Frees every node and resets the stack; values survive | O(n) |
| `isEmpty_stack()` | Tests whether the stack is empty | O(1) |
| `stack_push()` | Adds a value to the top | O(1) |
| `stack_pop()` | Removes and returns the top value | O(1) |
| `stack_top()` | Returns the top value without removing it | O(1) |

## Build and test

From this directory:

```sh
make test
make sanitize
make clean
```

The suite covers initialization, LIFO ordering, top without removal,
interleaved operations, `NULL` elements, clear and reuse, element ownership,
internal size consistency, and failure on empty pop/top operations.
