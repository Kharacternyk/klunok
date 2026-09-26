#include "timestamp.h"
#include "messages.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void test_timestamp(struct trace *trace) {
  char *timestamp = get_timestamp(0, "abc", 7, trace);
  assert(ok(trace));
  assert(!strcmp(timestamp, "abc"));
  free(timestamp);

  timestamp = get_timestamp(0, "abc", 3, trace);
  assert(ok(trace));
  assert(!strcmp(timestamp, "abc"));
  free(timestamp);

  timestamp = get_timestamp(0, "%C", 2, trace);
  assert(ok(trace));
  assert(strlen(timestamp) == 2);
  free(timestamp);

  timestamp = get_timestamp(0, "%Y%m%d%H%M%S", 14, trace);
  assert(ok(trace));
  assert(strlen(timestamp) == 14);
  free(timestamp);

  timestamp = get_timestamp(1000000000, "%Y%m%d%H%M%S", 14, trace);
  assert(ok(trace));
  assert(strlen(timestamp) == 14);
  free(timestamp);

  try(trace);
  timestamp = get_timestamp(0, "%Y", 3, trace);
  assert(catch_static(messages.timestamp.overflow, trace));
  finally(trace);

  free(timestamp);
}
