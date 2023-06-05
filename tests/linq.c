#include "linq.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define F1 TEST_ROOT "/meson.build"
#define F2 TEST_ROOT "/linq.c"

void test_linq(struct trace *trace) {
  struct linq *linq = load_linq("linq/instant", 0, 0, 0, trace);
  assert(ok(trace));

  struct linq_head *head = get_head(linq, trace);
  assert(ok(trace));
  assert(get_pause(head) < 0);
  free_linq_head(head);

  push(F1, 0, linq, trace);
  assert(ok(trace));
  head = get_head(linq, trace);
  assert(ok(trace));
  assert(!get_pause(head));
  assert(!strcmp(get_path(head), F1));
  assert(!get_metadata(head));
  free_linq_head(head);

  pop_head(linq, trace);
  assert(ok(trace));

  push(F1, 123, linq, trace);
  assert(ok(trace));
  push(F2, 1234, linq, trace);
  assert(ok(trace));
  push(F1, 12345, linq, trace);
  assert(ok(trace));

  head = get_head(linq, trace);
  assert(ok(trace));
  assert(!get_pause(head));
  assert(!strcmp(get_path(head), F2));
  assert(get_metadata(head) == 1234);
  free_linq_head(head);

  pop_head(linq, trace);
  assert(ok(trace));

  head = get_head(linq, trace);
  assert(ok(trace));
  assert(!get_pause(head));
  assert(!strcmp(get_path(head), F1));
  assert(get_metadata(head) == 12345);
  free_linq_head(head);

  pop_head(linq, trace);
  assert(ok(trace));

  head = get_head(linq, trace);
  assert(ok(trace));
  assert(get_pause(head) < 0);
  free_linq_head(head);

  free_linq(linq);

  linq = load_linq("linq/lag", 3600, 0, 0, trace);
  push(F1, 0, linq, trace);
  assert(ok(trace));
  head = get_head(linq, trace);
  assert(ok(trace));
  assert(get_pause(head) > 0);
  free_linq_head(head);
  pop_head(linq, trace);
  assert(ok(trace));

  free_linq(linq);
}
