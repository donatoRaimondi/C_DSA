#ifndef ORDERED_LIST_H
#define ORDERED_LIST_H
#include "../list/list.h"

typedef struct {
  list data;
  list_compare_fn compare;
} ordered_list;

/* Create an empty ordered list. compare must be non-NULL and remain valid. */
ordered_list ordered_list_init(list_compare_fn compare);

/* Free all nodes, retain the comparator, and reset the list. O(n). */
void ordered_list_clear(ordered_list *ol);

/* Return whether the list is empty. O(1). */
bool isEmpty_ordered_list(const ordered_list *ol);

/* Return the first position, or NULL when empty. O(1). */
pos ordered_list_first(const ordered_list *ol);

/* Return whether index is the end position (NULL). O(1). */
bool ordered_list_end(const ordered_list *ol, pos index);

/* Return the following position; NULL is invalid input. O(1). */
pos ordered_list_next(const ordered_list *ol, pos index);

/* Return the preceding position, or NULL when none is found. O(n). */
pos ordered_list_previous(const ordered_list *ol, pos index);

/* Insert at the comparator-defined sorted position. O(n). */
void ordered_list_insert(ordered_list *ol, elem value);

/* Remove a valid node, but not its stored value. O(n). */
void ordered_list_remove(ordered_list *ol, pos index);

/* Read the value at a valid non-end position. O(1). */
elem ordered_list_read(const ordered_list *ol, pos index);

/* Remove adjacent comparator-equivalent duplicates. O(n). */
void ordered_list_deduplicate(ordered_list *ol);

/* Return a new list containing both inputs, which must share a comparator. */
ordered_list ordered_list_fusion(const ordered_list *ol1,
                                 const ordered_list *ol2);

/* Return whether a comparator-equivalent value exists. O(n). */
bool ordered_list_search(const ordered_list *ol, elem key);

#endif // ! ORDERED_LIST_H
