#include "counter.h"
#include "trace.h"
#include <assert.h>

void test_counter() {
  struct trace *trace = create_trace();
  const char *path = "counter";
  size_t counter = 42;

  assert(read_counter(path, trace) == 0);
  assert(ok(trace));
  write_counter(path, counter, trace);
  assert(ok(trace));
  assert(read_counter(path, trace) == counter);

  free(trace);
}
