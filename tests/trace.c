#include "trace.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

void test_trace(struct trace *trace) {
  const char *a = "abc";
  const char *b = "XYZ";
  const char *c = "///";

  throw_static(a, trace);
  assert(!catch_static(b, trace));
  assert(catch_static(a, trace));

  throw_dynamic(b, trace);
  assert(!catch_static(b, trace));
  catch_all(trace);

  throw_context(c, trace);
  assert(!catch_static(c, trace));
  catch_all(trace);

  assert(ok(trace));
  errno = ENOMEM;
  throw_errno(trace);
  assert(!ok(trace));
  catch_all(trace);

  throw_static(a, trace);
  assert(!catch_static(b, trace));
  throw_static(b, trace);
  assert(!catch_static(a, trace));
  throw_static(a, trace);
  assert(!catch_static(b, trace));
  assert(catch_static(a, trace));
  catch_all(trace);
  assert(ok(trace));

  rethrow_check(trace);
  rethrow_check(trace);
  rethrow_static(a, trace);
  rethrow_static(b, trace);
  assert(ok(trace));

  rethrow_check(trace);
  rethrow_check(trace);
  throw_static(a, trace);
  rethrow_static(b, trace);
  rethrow_static(c, trace);
  assert(!catch_static(a, trace));
  assert(!catch_static(b, trace));
  assert(catch_static(c, trace));

  rethrow_check(trace);
  throw_static(a, trace);
  rethrow_check(trace);
  rethrow_static(b, trace);
  rethrow_static(c, trace);
  assert(!catch_static(a, trace));
  assert(!catch_static(b, trace));
  assert(catch_static(c, trace));

  throw_static(a, trace);
  rethrow_check(trace);
  rethrow_check(trace);
  rethrow_static(b, trace);
  rethrow_static(c, trace);
  assert(!catch_static(b, trace));
  assert(!catch_static(c, trace));
  assert(catch_static(a, trace));
}
