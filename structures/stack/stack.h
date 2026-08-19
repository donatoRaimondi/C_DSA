#ifndef STACK_H
#define STACK_H

#include "../list/list.h"

typedef struct stack {
  list data;
} stack;

/* Create an empty stack. O(1). */
stack stack_init(void);

/* Free all stack nodes; stored elem values are not freed. O(n). */
void stack_clear(stack *stack);

/* Return whether the stack contains no elements. O(1). */
bool isEmpty_stack(const stack *stack);

/* Push value onto the top. O(1). */
void stack_push(stack *stack, elem value);

/* Remove and return the top value. The stack must be nonempty. O(1). */
elem stack_pop(stack *stack);

/* Return the top value without removing it. The stack must be nonempty. O(1). */
elem stack_top(const stack *stack);

#endif // ! STACK_H
