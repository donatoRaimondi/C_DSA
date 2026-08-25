#include "dictionary.h"
#include <err.h>
#include <stdbool.h>
#include <stdlib.h>

dictionary dictionary_init(size_t capacity, dictionary_hash_fn hash,
                           dictionary_equal_fn equal) {
  if (hash == NULL || equal == NULL)
    errx(EXIT_FAILURE, "dictionary_init: hash and equal must be non-NULL");

  if (capacity <= 0)
    errx(EXIT_FAILURE, "dictionary_init: capacity must be > 0");

  dictionary d = {.slots = calloc(capacity, sizeof *d.slots),
                  .size = 0,
                  .capacity = capacity,
                  .hash = hash,
                  .equal = equal};
  if (d.slots == NULL)
    err(EXIT_FAILURE, "dictionary_init: allocation failued");

  return d;
}

void dictionary_clear(dictionary *d) {
  free(d->slots);

  d->slots = NULL;
  d->size = 0;
  d->capacity = 0;
  d->equal = NULL;
  d->hash = NULL;
}

bool isEmpty_dictionary(const dictionary *d) { return d->size == 0; }

static size_t probe_index(const dictionary *d, elem key, size_t i) {
  size_t h0 = d->hash(key) % d->capacity;
  return (h0 + i) % d->capacity;
}

bool dictionary_contains(const dictionary *d, elem key) {
  if (d->capacity == 0)
    errx(EXIT_FAILURE, "dictionary_contains: capacity must be > 0");

  if (isEmpty_dictionary(d))
    return false;

  for (size_t i = 0; i < d->capacity; i++) {
    size_t h = probe_index(d, key, i);

    if (d->slots[h].state == SLOT_EMPTY)
      return false;

    if (d->slots[h].state == SLOT_OCCUPIED && d->equal(d->slots[h].key, key))
      return true;
  }

  return false;
}

void dictionary_insert(dictionary *d, elem key, elem value) {
  if (d->capacity == 0)
    errx(EXIT_FAILURE, "dictionary_insert: capacity must be > 0");

  size_t first_deleted = SIZE_MAX;

  for (size_t i = 0; i < d->capacity; i++) {
    size_t h = probe_index(d, key, i);

    if (d->slots[h].state == SLOT_OCCUPIED) {
      if (d->equal(d->slots[h].key, key)) {
        d->slots[h].value = value;
        return;
      }

      continue;
    }

    if (d->slots[h].state == SLOT_DELETED) {
      if (first_deleted == SIZE_MAX)
        first_deleted = h;

      continue;
    }

    // SLOT_EMPTY
    size_t target = first_deleted != SIZE_MAX ? first_deleted : h;

    d->slots[target].key = key;
    d->slots[target].value = value;
    d->slots[target].state = SLOT_OCCUPIED;
    d->size++;
    return;
  }

  if (first_deleted != SIZE_MAX) {
    d->slots[first_deleted].key = key;
    d->slots[first_deleted].value = value;
    d->slots[first_deleted].state = SLOT_OCCUPIED;
    d->size++;
    return;
  }

  errx(EXIT_FAILURE, "dictionary_insert: dictionary is full");
}

void dictionary_remove(dictionary *d, elem key) {
  if (d->capacity == 0)
    errx(EXIT_FAILURE, "dictionary_remove: capacity must be > 0");

  for (size_t i = 0; i < d->capacity; i++) {
    size_t h = probe_index(d, key, i);

    if (d->slots[h].state == SLOT_EMPTY)
      return;

    if (d->slots[h].state == SLOT_OCCUPIED && d->equal(d->slots[h].key, key)) {
      d->slots[h].state = SLOT_DELETED;
      d->slots[h].value = NULL;
      d->slots[h].key = NULL;
      d->size--;
      return;
    }
  }
}

elem dictionary_get(const dictionary *d, elem key) {
  if (d->capacity == 0)
    errx(EXIT_FAILURE, "dictionary_get: capacity must be > 0");

  if (isEmpty_dictionary(d))
    errx(EXIT_FAILURE, "dictionary_get: dictionary is empty");

  for (size_t i = 0; i < d->capacity; i++) {
    size_t h = probe_index(d, key, i);

    if (d->slots[h].state == SLOT_EMPTY)
      errx(EXIT_FAILURE, "dictionary_get: key not present");

    if (d->slots[h].state == SLOT_OCCUPIED && d->equal(d->slots[h].key, key))
      return d->slots[h].value;
  }

  errx(EXIT_FAILURE, "dictionary_get: key not present");
}
