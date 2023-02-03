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
  const char *c = "///";

  throw_static(a, trace);
  throw_dynamic(b, trace);
  throw_context(c, trace);
  errno = ENOMEM;
  throw_errno(trace);

  assert(!ok(trace));

  assert(get_trace_message(trace));
  assert(!strcmp(get_trace_message(trace), strerror(ENOMEM)));
  assert(!is_trace_message_context(trace));

  pop_trace_message(trace);
  assert(!strcmp(get_trace_message(trace), c));
  assert(get_trace_message(trace) != c);
  assert(is_trace_message_context(trace));

  pop_trace_message(trace);
  assert(!strcmp(get_trace_message(trace), b));
  assert(get_trace_message(trace) != b);
  assert(!is_trace_message_context(trace));

  pop_trace_message(trace);
  assert(!strcmp(get_trace_message(trace), a));
  assert(get_trace_message(trace) == a);
  assert(!is_trace_message_context(trace));

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

  rethrow_check(trace);
  throw_static(a, trace);
  rethrow_context(c, trace);
  rethrow_static(b, trace);
  pop_trace_message(trace);
  assert(is_trace_message_context(trace));
  catch_all(trace);

  rethrow_check(trace);
  rethrow_context(c, trace);
  rethrow_static(b, trace);
  assert(ok(trace));

  free(trace);
}
