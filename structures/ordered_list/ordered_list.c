#include "ordered_list.h"
#include <err.h>
#include <stdlib.h>

ordered_list ordered_list_init(list_compare_fn compare) {
  ordered_list ol = {.data = list_init(), .compare = compare};

  return ol;
}

void ordered_list_clear(ordered_list *ol) { list_clear(&ol->data); }

bool isEmpty_ordered_list(const ordered_list *ol) {
  return isEmpty_list(&ol->data);
}

pos ordered_list_first(const ordered_list *ol) { return list_first(&ol->data); }

bool ordered_list_end(const ordered_list *ol, pos index) {
  return list_end(&ol->data, index);
}

pos ordered_list_next(const ordered_list *ol, pos index) {
  return list_next(&ol->data, index);
}

pos ordered_list_previous(const ordered_list *ol, pos index) {
  return list_previous(&ol->data, index);
}

// method that doesn't need the index since the list is ordered
void ordered_list_insert(ordered_list *ol, elem value) {
  pos curr = ordered_list_first(ol);
  while (!ordered_list_end(ol, curr)) {
    if (ol->compare(value, curr->value) <= 0) {
      list_insert(&ol->data, value, curr);
      return;
    }
    curr = ordered_list_next(ol, curr);
  }
  list_insert(&ol->data, value, NULL);
}

void ordered_list_remove(ordered_list *ol, pos index) {
  list_remove(&ol->data, index);
}

elem ordered_list_read(const ordered_list *ol, pos index) {
  return list_read(&ol->data, index);
}

// ordered_list_write is not permitted

void ordered_list_deduplicate(ordered_list *ol) {
  pos curr = ordered_list_first(ol);

  while (!ordered_list_end(ol, curr)) {
    pos next = ordered_list_next(ol, curr);

    if (!ordered_list_end(ol, next) &&
        ol->compare(ordered_list_read(ol, curr), ordered_list_read(ol, next)) ==
            0) {
      ordered_list_remove(ol, next);
    } else {
      curr = next;
    }
  }
}

ordered_list ordered_list_fusion(const ordered_list *ol1,
                                 const ordered_list *ol2) {
  if (ol1->compare != ol2->compare) {
    errx(EXIT_FAILURE, "ordered_list_fusion: incompatible compare functions");
  }

  ordered_list result = {.data =
                             list_fusion(&ol1->data, &ol2->data, ol1->compare),
                         .compare = ol1->compare};

  return result;
}

bool ordered_list_search(const ordered_list *ol, elem key) {
  return list_search(&ol->data, key, ol->compare);
}
