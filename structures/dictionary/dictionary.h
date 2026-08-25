#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <stdbool.h>
#include <stddef.h>

typedef void *elem;

typedef enum { SLOT_EMPTY, SLOT_OCCUPIED, SLOT_DELETED } slot_state;

typedef size_t (*dictionary_hash_fn)(const elem value);

typedef bool (*dictionary_equal_fn)(const elem left, const elem right);

typedef struct {
  elem value;
  elem key;
  slot_state state;
} dictionary_slot;

typedef struct {
  dictionary_slot *slots;

  size_t size;
  size_t capacity;

  dictionary_hash_fn hash;
  dictionary_equal_fn equal;
} dictionary;

dictionary dictionary_init(size_t capacity, dictionary_hash_fn hash,
                           dictionary_equal_fn equal);

void dictionary_clear(dictionary *d);

bool isEmpty_dictionary(const dictionary *d);

bool dictionary_contains(const dictionary *d, elem key);

void dictionary_insert(dictionary *d, elem key, elem value);

void dictionary_remove(dictionary *d, elem key);

elem dictionary_get(const dictionary *d, elem key);

#endif // ! DICTIONARY_H
