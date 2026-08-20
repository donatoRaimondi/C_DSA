#include "set.h"
#include <err.h>
#include <stddef.h>
#include <stdlib.h>

set set_init(size_t capacity, set_hash_fn hash, set_equal_fn equal) {
  if (capacity == 0)
    errx(EXIT_FAILURE, "set_init: capacity must be > 0");
  if (hash == NULL || equal == NULL)
    errx(EXIT_FAILURE, "set_init: hash and equal must be non-NULL");

  set s = {.slots = calloc(capacity, sizeof *s.slots),
           .size = 0,
           .capacity = capacity,
           .hash = hash,
           .equal = equal};

  if (s.slots == NULL)
    err(EXIT_FAILURE, "set_init: allocation failed");

  return s;
}

void set_clear(set *s) {
  free(s->slots);

  s->slots = NULL;
  s->size = 0;
  s->capacity = 0;
  s->hash = NULL;
  s->equal = NULL;
}

bool isEmpty_set(const set *s) { return s->size == 0; }

static size_t probe_index(const set *s, elem key, size_t i) {
  size_t h0 = s->hash(key) % s->capacity;
  return (h0 + i) % s->capacity;
}

bool set_contains(const set *s, elem key) {
  if (isEmpty_set(s))
    return false;

  if (s->capacity == 0)
    errx(EXIT_FAILURE, "set_init: capacity must be > 0");

  for (size_t i = 0; i < s->capacity; i++) {
    size_t h = probe_index(s, key, i);

    if (s->slots[h].state == SLOT_EMPTY)
      return false;

    if (s->slots[h].state == SLOT_OCCUPIED && s->equal(s->slots[h].value, key))
      return true;
  }

  return false;
}

void set_insert(set *s, elem key) {
  if (s->capacity == 0)
    errx(EXIT_FAILURE, "set_insert: capacity must be > 0");

  size_t first_deleted = SIZE_MAX;

  for (size_t i = 0; i < s->capacity; i++) {
    size_t h = probe_index(s, key, i);

    if (s->slots[h].state == SLOT_OCCUPIED) {
      if (s->equal(s->slots[h].value, key))
        return; // already present

      continue;
    }

    if (s->slots[h].state == SLOT_DELETED) {
      if (first_deleted == SIZE_MAX)
        first_deleted = h;

      continue;
    }

    // SLOT_EMPTY
    size_t target = first_deleted != SIZE_MAX ? first_deleted : h;

    s->slots[target].value = key;
    s->slots[target].state = SLOT_OCCUPIED;
    s->size++;
    return;
  }

  // No EMPTY slot found, but there may be a tombstone.
  if (first_deleted != SIZE_MAX) {
    s->slots[first_deleted].value = key;
    s->slots[first_deleted].state = SLOT_OCCUPIED;
    s->size++;
    return;
  }

  errx(EXIT_FAILURE, "set_insert: set is full");
}

void set_remove(set *s, elem key) {
  for (size_t i = 0; i < s->capacity; i++) {
    size_t h = probe_index(s, key, i);

    if (s->slots[h].state == SLOT_EMPTY)
      return;

    if (s->slots[h].state == SLOT_OCCUPIED &&
        s->equal(s->slots[h].value, key)) {
      s->slots[h].state = SLOT_DELETED;
      s->slots[h].value = NULL;
      s->size--;
      return;
    }
  }
}

set set_union(const set *set_a, const set *set_b) {
  if (set_a->hash != set_b->hash || set_a->equal != set_b->equal)
    errx(EXIT_FAILURE, "set_union: incompatible sets");

  set s = set_init(set_a->capacity + set_b->capacity,
                   set_a->hash, set_a->equal);

  for (size_t i = 0; i < set_a->capacity; i++) {
    if (set_a->slots[i].state == SLOT_OCCUPIED)
      set_insert(&s, set_a->slots[i].value);
  }
  for (size_t i = 0; i < set_b->capacity; i++) {
    if (set_b->slots[i].state == SLOT_OCCUPIED)
      set_insert(&s, set_b->slots[i].value);
  }

  return s;
}

set set_intersect(const set *set_a, const set *set_b) {
  if (set_a->hash != set_b->hash || set_a->equal != set_b->equal)
    errx(EXIT_FAILURE, "set_intersect: incompatible sets");

  set s =
      set_init(set_a->capacity + set_b->capacity, set_a->hash, set_a->equal);

  for (size_t i = 0; i < set_a->capacity; i++) {
    if (set_a->slots[i].state == SLOT_OCCUPIED &&
        set_contains(set_b, set_a->slots[i].value))
      set_insert(&s, set_a->slots[i].value);
  }

  return s;
}

set set_difference(const set *set_a, const set *set_b) {
  if (set_a->hash != set_b->hash || set_a->equal != set_b->equal)
    errx(EXIT_FAILURE, "set_difference: incompatible sets");

  set s = set_init(set_a->capacity, set_a->hash, set_a->equal);

  for (size_t i = 0; i < set_a->capacity; i++) {
    if (set_a->slots[i].state == SLOT_OCCUPIED &&
        !set_contains(set_b, set_a->slots[i].value))
      set_insert(&s, set_a->slots[i].value);
  }

  return s;
}
