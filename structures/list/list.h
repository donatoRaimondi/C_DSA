#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <stddef.h>

/* The list stores pointers but never allocates or frees the pointed-to value. */
typedef void *elem;

typedef struct node {
  elem value;
  struct node *next;
} node;

typedef node *pos;

/* Return < 0, 0, or > 0 when left precedes, equals, or follows right. */
typedef int (*list_compare_fn)(const elem left, const elem right);

typedef struct list {
  size_t size;
  pos head;
} list;

/* Create an empty list. O(1). */
list list_init(void);

/* Free all nodes and reset the list; stored elem values are not freed. O(n). */
void list_clear(list *l);

/* Return whether the list contains no nodes. O(1). */
bool isEmpty_list(const list *list);

/* Return the first position, or NULL when empty. O(1). */
pos list_first(const list *list);

/* Return whether index is the end position (NULL). O(1). */
bool list_end(const list *list, pos index);

/* Return the following position; NULL is invalid input. O(1). */
pos list_next(const list *list, pos index);

/* Return the preceding position, or NULL when none is found. O(n). */
pos list_previous(const list *list, pos index);

/* Insert before index; NULL appends. O(1) at the head, O(n) otherwise. */
void list_insert(list *list, elem value, pos index);

/* Remove a valid node, but not its value. O(1) at the head, O(n) otherwise. */
void list_remove(list *list, pos index);

/* Read the value at a valid non-end position. O(1). */
elem list_read(const list *list, pos index);

/* Replace the value at a valid non-end position. O(1). */
void list_write(list *list, elem value, pos index);

/* Remove comparator-equivalent duplicates, keeping first occurrences. O(n^2). */
void list_deduplicate(list *list, list_compare_fn compare);

/* Return a new list containing two sorted lists; element pointers are shared. */
list list_fusion(const list *l1, const list *l2, list_compare_fn compare);

/* Stable natural merge sort; existing nodes are relinked, not reallocated. */
void list_natural_mergesort(list *l, list_compare_fn compare);

/* Return whether a comparator-equivalent value exists. O(n). */
bool list_search(const list *l, elem key, list_compare_fn compare);

#endif // !LIST_H
