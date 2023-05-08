#include "list.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void test_list(struct trace *trace) {
  struct list *list = create_list(trace);
  assert(ok(trace));
  assert(!peek(list));

  const char *head = "HEAD";
  join(head, list, trace);
  assert(ok(trace));

  const struct list_item *head_item = peek(list);
  assert(head_item);
  assert(!get_next(head_item));
  assert(get_value(head_item) != head);
  assert(!strcmp(get_value(head_item), head));

  const char *next_head = "NEXT";
  join(next_head, list, trace);
  assert(ok(trace));
  assert(peek(list));
  assert(get_next(peek(list)) == head_item);
  assert(get_value(peek(list)) != next_head);
  assert(!strcmp(get_value(peek(list)), next_head));

  free_list(list);
}
