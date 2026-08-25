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

/* Create an empty fixed-capacity dictionary. Arguments must be valid. */
dictionary dictionary_init(size_t capacity, dictionary_hash_fn hash,
                           dictionary_equal_fn equal);

/* Free the slot array and reset all fields; keys and values are not freed. */
void dictionary_clear(dictionary *d);

/* Return whether the dictionary contains no key/value pairs. O(1). */
bool isEmpty_dictionary(const dictionary *d);

/* Test whether an equivalent key exists. Average O(1), worst O(capacity). */
bool dictionary_contains(const dictionary *d, elem key);

/* Insert a pair or replace the value for an equivalent key. */
void dictionary_insert(dictionary *d, elem key, elem value);

/* Remove an equivalent key if present; missing keys have no effect. */
void dictionary_remove(dictionary *d, elem key);

/* Return the value for key; terminates when the key is absent. */
elem dictionary_get(const dictionary *d, elem key);

#endif // ! DICTIONARY_H
