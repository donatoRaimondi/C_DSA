#include "list.h"
#include <err.h>
#include <stdbool.h>
#include <stdlib.h>

list list_init(void) {
  list l = {.size = 0, .head = NULL};

  return l;
}

void list_clear(list *l) {
  pos curr = list_first(l);

  while (!list_end(l, curr)) {
    pos next = list_next(l, curr);
    free(curr);
    curr = next;
  }

  l->head = NULL;
  l->size = 0;
}

bool isEmpty_list(const list *list) { return list->size == 0; }

pos list_first(const list *list) { return list->head; }

bool list_end(const list *list, pos index) {
  (void)list;
  return index == NULL;
}

pos list_next(const list *list, pos index) {
  if (!list_end(list, index)) {
    return index->next;
  }
  errx(EXIT_FAILURE, "list_next: invalid position");
}

pos list_previous(const list *list, pos index) {
  if (index == list_first(list) || list_end(list, index))
    return NULL;

  pos curr = list_first(list);

  while (!list_end(list, curr) && list_next(list, curr) != index) {
    curr = list_next(list, curr);
  }

  return curr;
}

void list_insert(list *list, elem value, pos index) {
  node *new = malloc(sizeof(node));

  if (new == NULL)
    err(EXIT_FAILURE, "list_insert: malloc failed");

  new->value = value;
  new->next = NULL;

  // inserting a new head
  if (index == list_first(list)) {
    new->next = list->head;
    list->head = new;
  }
  // inserting at the tail of the list - it should be safe to have this
  // condition since if the list is empty, the previous case should make
  // possible to insert in the head which is NULL
  else if (index == NULL) {
    pos curr = list_first(list);
    while (!list_end(list, curr->next)) {
      curr = curr->next;
    }
    curr->next = new;
  }
  // inserting in the middle of the list using list_previous
  else {
    pos prev = list_previous(list, index);

    if (prev == NULL) {
      free(new);
      errx(EXIT_FAILURE, "list_insert: invalid position");
    }
    prev->next = new;
    new->next = index;
  }

  list->size++;
}

void list_remove(list *list, pos index) {
  if (isEmpty_list(list))
    errx(EXIT_FAILURE, "list_remove: list is empty");

  // remove the head
  if (index == list_first(list)) {
    list->head = list->head->next;
    free(index);
  }
  // remove the tail and in the middle
  else {
    pos prev = list_previous(list, index);

    if (prev == NULL)
      errx(EXIT_FAILURE, "list_remove: invalid position");

    prev->next = index->next;
    free(index);
  }

  list->size--;
}

elem list_read(const list *list, pos index) {
  if (isEmpty_list(list))
    errx(EXIT_FAILURE, "list_read: list is empty");

  if (index == NULL)
    errx(EXIT_FAILURE, "list_read: invalid position");

  return index->value;
}

void list_write(list *list, elem value, pos index) {
  if (isEmpty_list(list))
    errx(EXIT_FAILURE, "list_write: list is empty");

  if (index == NULL)
    errx(EXIT_FAILURE, "list_write: invalid position");

  index->value = value;
}

void list_deduplicate(list *list, list_compare_fn compare) {
  if (compare == NULL)
    errx(EXIT_FAILURE, "list_deduplicate: compare is NULL");

  pos curr = list_first(list);

  while (!list_end(list, curr)) {
    pos succ = list_next(list, curr);

    while (!list_end(list, succ)) {
      if (compare(list_read(list, curr), list_read(list, succ)) == 0) {
        pos next = list_next(list, succ);
        list_remove(list, succ);
        succ = next;
      } else
        succ = list_next(list, succ);
    }

    curr = list_next(list, curr);
  }
}

list list_fusion(const list *l1, const list *l2, list_compare_fn compare) {
  list result_list = list_init();

  if (compare == NULL)
    errx(EXIT_FAILURE, "list_fusion: compare is NULL");

  elem e1, e2;
  pos p1 = list_first(l1);
  pos p2 = list_first(l2);

  while (!list_end(l1, p1) && !list_end(l2, p2)) {
    e1 = list_read(l1, p1);
    e2 = list_read(l2, p2);

    if (compare(e1, e2) <= 0) {
      list_insert(&result_list, e1, NULL);
      p1 = list_next(l1, p1);
    } else {
      list_insert(&result_list, e2, NULL);
      p2 = list_next(l2, p2);
    }
  }

  while (!list_end(l1, p1)) {
    list_insert(&result_list, list_read(l1, p1), NULL);
    p1 = list_next(l1, p1);
  }
  while (!list_end(l2, p2)) {
    list_insert(&result_list, list_read(l2, p2), NULL);
    p2 = list_next(l2, p2);
  }

  return result_list;
}

static node *merge_runs(node *left, node *right, list_compare_fn compare,
                        node **tail) {
  node dummy = {.value = NULL, .next = NULL};
  node *out = &dummy;

  while (left != NULL && right != NULL) {
    /* Taking from the left on equality makes the sort stable. */
    if (compare(left->value, right->value) <= 0) {
      out->next = left;
      left = left->next;
    } else {
      out->next = right;
      right = right->next;
    }
    out = out->next;
  }

  out->next = left != NULL ? left : right;
  while (out->next != NULL)
    out = out->next;

  *tail = out;
  return dummy.next;
}

void list_natural_mergesort(list *l, list_compare_fn compare) {
  if (compare == NULL)
    errx(EXIT_FAILURE, "list_natural_mergesort: compare is NULL");

  if (l->head == NULL || l->head->next == NULL)
    return;

  for (;;) {
    node *remaining = l->head;
    node *sorted_head = NULL;
    node *sorted_tail = NULL;
    size_t merges = 0;

    while (remaining != NULL) {
      node *left = remaining;
      node *left_tail = left;

      while (left_tail->next != NULL &&
             compare(left_tail->value, left_tail->next->value) <= 0)
        left_tail = left_tail->next;

      node *right = left_tail->next;
      left_tail->next = NULL;

      if (right == NULL) {
        if (sorted_head == NULL)
          sorted_head = left;
        else
          sorted_tail->next = left;
        sorted_tail = left_tail;
        remaining = NULL;
        continue;
      }

      node *right_tail = right;
      while (right_tail->next != NULL &&
             compare(right_tail->value, right_tail->next->value) <= 0)
        right_tail = right_tail->next;

      remaining = right_tail->next;
      right_tail->next = NULL;

      node *merged_tail = NULL;
      node *merged = merge_runs(left, right, compare, &merged_tail);
      if (sorted_head == NULL)
        sorted_head = merged;
      else
        sorted_tail->next = merged;
      sorted_tail = merged_tail;
      ++merges;
    }

    l->head = sorted_head;
    if (merges == 0)
      return;
  }
}

bool list_search(const list *l, elem key, list_compare_fn compare) {
  pos curr = list_first(l);

  while (!list_end(l, curr)) {
    if (compare(key, curr->value) == 0)
      return true;
    curr = list_next(l, curr);
  }
  return false;
}
