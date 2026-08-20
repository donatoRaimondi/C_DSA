#ifndef SET_H
#define SET_H

// #include "../list/list.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

typedef void *elem;

typedef enum { SLOT_EMPTY, SLOT_OCCUPIED, SLOT_DELETED } slot_state;

typedef size_t (*set_hash_fn)(const elem value);

typedef bool (*set_equal_fn)(const elem left, const elem right);

typedef struct {
  elem value;
  slot_state state;
} slot;

typedef struct {
  slot *slots;

  size_t size;
  size_t capacity;

  set_hash_fn hash;
  set_equal_fn equal;
} set;

/* Create an empty fixed-capacity set. All arguments must be valid. O(capacity). */
set set_init(size_t capacity, set_hash_fn hash, set_equal_fn equal);

/* Free the slot array and reset every field. Stored values are not freed. O(1). */
void set_clear(set *set);

/* Return whether the set contains no values. O(1). */
bool isEmpty_set(const set *set);

/* Test membership using hash and equality callbacks. Average O(1), worst O(n). */
bool set_contains(const set *set, elem key);

/* Insert key if absent. Terminates if the fixed-capacity table is full. */
void set_insert(set *set, elem key);

/* Remove key if present; removing a missing key has no effect. */
void set_remove(set *set, elem key);

/* Return A ∪ B. Inputs must use the exact same callbacks. */
set set_union(const set *set_a, const set *set_b);

/* Return A ∩ B. Inputs must use the exact same callbacks. */
set set_intersect(const set *set_a, const set *set_b);

/* Return A − B. Inputs must use the exact same callbacks. */
set set_difference(const set *set_a, const set *set_b);

#endif // ! SET_H
