#include "list.h"
#include "trace.h"
#include <stdlib.h>
#include <string.h>

struct list_item {
  char *value;
  struct list_item *next;
};

struct list {
  struct list_item *head;
};

struct list *create_list(struct trace *trace) {
  struct list *list = TNULL(malloc(sizeof(struct list)), trace);
  if (!ok(trace)) {
    return NULL;
  }
  list->head = NULL;
  return list;
}

void join(const char *head, struct list *tail, struct trace *trace) {
  struct list_item *item = TNULL(malloc(sizeof(struct list_item)), trace);
  char *value = TNULL(strdup(head), trace);
  if (!ok(trace)) {
    return free(item);
  }
  item->value = value;
  item->next = tail->head;
  tail->head = item;
}

const struct list_item *peek(const struct list *list) { return list->head; }

const struct list_item *get_next(const struct list_item *item) {
  return item->next;
}

const char *get_value(const struct list_item *item) { return item->value; }

void free_list(struct list *list) {
  if (list) {
    while (list->head) {
      struct list_item *item = list->head;
      list->head = item->next;
      free(item->value);
      free(item);
    }
    free(list);
  }
}
