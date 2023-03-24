#include "linq.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define INSTANT_LINQ_PATH "./linq/instant"
#define LAGGED_LINQ_PATH "./linq/lagged"
#define F1 TEST_ROOT "/meson.build"
#define F2 TEST_ROOT "/linq.c"

void test_linq() {
  struct trace *trace = create_trace();
  struct linq *linq = load_linq(INSTANT_LINQ_PATH, 0, 0, 0, trace);
  assert(ok(trace));

  time_t retry_after_seconds = 0;
  char *path = get_linq_head(linq, &retry_after_seconds, trace);
  assert(ok(trace));
  assert(retry_after_seconds < 0);
  retry_after_seconds = 0;

  push_to_linq(F1, linq, trace);
  assert(ok(trace));
  path = get_linq_head(linq, &retry_after_seconds, trace);
  assert(ok(trace));
  assert(!retry_after_seconds);
  assert(!strcmp(path, F1));
  free(path);

  pop_from_linq(linq, trace);
  assert(ok(trace));

  push_to_linq(F1, linq, trace);
  assert(ok(trace));
  push_to_linq(F2, linq, trace);
  assert(ok(trace));
  push_to_linq(F1, linq, trace);
  assert(ok(trace));

  path = get_linq_head(linq, &retry_after_seconds, trace);
  assert(ok(trace));
  assert(!retry_after_seconds);
  assert(!strcmp(path, F2));
  free(path);

  pop_from_linq(linq, trace);
  assert(ok(trace));

  path = get_linq_head(linq, &retry_after_seconds, trace);
  assert(ok(trace));
  assert(!retry_after_seconds);
  assert(!strcmp(path, F1));
  free(path);

  pop_from_linq(linq, trace);
  assert(ok(trace));

  get_linq_head(linq, &retry_after_seconds, trace);
  assert(ok(trace));
  assert(retry_after_seconds < 0);

  free_linq(linq);

  linq = load_linq(LAGGED_LINQ_PATH, 3600, 0, 0, trace);
  push_to_linq(F1, linq, trace);
  assert(ok(trace));
  path = get_linq_head(linq, &retry_after_seconds, trace);
  assert(ok(trace));
  assert(retry_after_seconds > 0);
  pop_from_linq(linq, trace);
  assert(ok(trace));

  free_linq(linq);
  free(trace);
}
