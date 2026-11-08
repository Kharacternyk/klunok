#include "linq.h"
#include "messages.h"
#include "test-constants.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define F1 TEST_ROOT "/meson.build"
#define F2 TEST_ROOT "/linq.c"

void test_linq(struct trace *trace) {
  char *f1 = strdup(F1);
  char *f2 = strdup(F2);

  assert(f1 && f2);

  struct linq *linq = load_linq("linq/instant", 0, 0, 0, trace);
  assert(ok(trace));

  struct linq_head *head = get_head(linq, trace);
  assert(ok(trace));
  assert(get_pause(head) < 0);
  free_linq_head(head);

  push(f1, linq, trace);
  assert(ok(trace));
  free_linq(linq);
  linq = load_linq("linq/instant", 0, 0, 0, trace);
  assert(ok(trace));
  head = get_head(linq, trace);
  assert(ok(trace));
  assert(!get_pause(head));
  assert(!strcmp(get_path(head), f1));
  free_linq_head(head);

  pop_head(linq, trace);
  assert(ok(trace));

  push(f1, linq, trace);
  assert(ok(trace));
  push(f2, linq, trace);
  assert(ok(trace));
  push(f1, linq, trace);
  assert(ok(trace));

  head = get_head(linq, trace);
  assert(ok(trace));
  assert(!get_pause(head));
  assert(!strcmp(get_path(head), f2));
  free_linq_head(head);

  pop_head(linq, trace);
  assert(ok(trace));

  head = get_head(linq, trace);
  assert(ok(trace));
  assert(!get_pause(head));
  assert(!strcmp(get_path(head), f1));
  free_linq_head(head);

  pop_head(linq, trace);
  assert(ok(trace));

  head = get_head(linq, trace);
  assert(ok(trace));
  assert(get_pause(head) < 0);
  free_linq_head(head);

  free_linq(linq);

  const char *legacy[] = {"//file", "/./file", "/.//./file"};
  for (size_t i = 0; i < sizeof(legacy) / sizeof(*legacy); ++i) {
    assert(!symlink(legacy[i], "linq/instant/0"));
    try(trace);
    assert(!load_linq("linq/instant", 0, 0, 0, trace));
    assert(catch_static(messages.linq.legacy_metadata, trace));
    finally(trace);
    assert(!unlink("linq/instant/0"));
  }

  linq = load_linq("linq/lag", 3600, 0, 0, trace);
  push(f1, linq, trace);
  assert(ok(trace));
  head = get_head(linq, trace);
  assert(ok(trace));
  assert(get_pause(head) > 0);
  free_linq_head(head);
  pop_head(linq, trace);
  assert(ok(trace));

  free_linq(linq);
  free(f1);
  free(f2);
}
