#include "trace.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

void test_trace() {
  struct trace *trace = create_trace();
  assert(trace);
  assert(!get_trace_message(trace));
  assert(ok(trace));

  const char *a = "abc";
  const char *b = "XYZ";

  throw_static(a, trace);
  throw_dynamic(b, trace);
  errno = ENOMEM;
  throw_errno(trace);

  assert(!ok(trace));

  assert(get_trace_message(trace));
  assert(!strcmp(get_trace_message(trace), strerror(ENOMEM)));
  pop_trace_message(trace);
  assert(!strcmp(get_trace_message(trace), b));
  assert(get_trace_message(trace) != b);
  pop_trace_message(trace);
  assert(!strcmp(get_trace_message(trace), a));
  assert(get_trace_message(trace) == a);
  pop_trace_message(trace);
  assert(!get_trace_message(trace));

  throw_static(a, trace);
  throw_static(b, trace);
  throw_static(a, trace);
  assert(!ok(trace));
  catch_all(trace);
  assert(ok(trace));

  throw_static(a, trace);
  throw_static(b, trace);
  throw_static(a, trace);
  assert(!ok(trace));
  assert(!catch_static(b, trace));
  assert(catch_static(a, trace));
  assert(ok(trace));

  rethrow_check(trace);
  rethrow_check(trace);
  rethrow_static(a, trace);
  rethrow_static(b, trace);
  assert(ok(trace));

  const char *c = "\\m/";

  rethrow_check(trace);
  rethrow_check(trace);
  throw_static(a, trace);
  assert(get_trace_message(trace) == a);
  rethrow_static(b, trace);
  assert(get_trace_message(trace) == b);
  rethrow_static(c, trace);
  assert(get_trace_message(trace) == c);
  assert(catch_static(c, trace));

  rethrow_check(trace);
  throw_static(a, trace);
  assert(get_trace_message(trace) == a);
  rethrow_check(trace);
  rethrow_static(b, trace);
  assert(get_trace_message(trace) == a);
  rethrow_static(c, trace);
  assert(get_trace_message(trace) == c);
  assert(catch_static(c, trace));

  throw_static(a, trace);
  assert(get_trace_message(trace) == a);
  rethrow_check(trace);
  rethrow_check(trace);
  rethrow_static(b, trace);
  assert(get_trace_message(trace) == a);
  rethrow_static(c, trace);
  assert(get_trace_message(trace) == a);
  assert(catch_static(a, trace));

  free(trace);
}
