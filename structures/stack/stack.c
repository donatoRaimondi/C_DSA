#include "stack.h"

stack stack_init(void) {
  stack s = {.data = list_init()};

  return s;
}

void stack_clear(stack *stack) { list_clear(&stack->data); }

bool isEmpty_stack(const stack *stack) { return isEmpty_list(&stack->data); }

void stack_push(stack *stack, elem value) {
  list_insert(&stack->data, value, list_first(&stack->data));
}

elem stack_pop(stack *stack) {
  pos first = list_first(&stack->data);
  elem value = list_read(&stack->data, first);
  list_remove(&stack->data, first);
  return value;
}

elem stack_top(const stack *stack) {
  pos first = list_first(&stack->data);
  elem value = list_read(&stack->data, first);
  return value;
}
