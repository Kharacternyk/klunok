#include "trace.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

void test_trace(struct trace *trace) {
  const char *a = "abc";
  const char *b = "XYZ";
  const char *c = "///";

  try(trace);
  throw_static(a, trace);
  assert(!catch_static(b, trace));
  assert(catch_static(a, trace));
  finally(trace);

  try(trace);
  throw_static(a, trace);
  try(trace);
  assert(!catch_static(a, trace));
  finally(trace);
  assert(catch_static(a, trace));
  finally(trace);

  try(trace);
  throw_dynamic(b, trace);
  assert(!catch_static(b, trace));
  finally_catch_all(trace);

  try(trace);
  throw_context(c, trace);
  assert(!catch_static(c, trace));
  finally_catch_all(trace);

  try(trace);
  assert(ok(trace));
  errno = ENOMEM;
  throw_errno(trace);
  assert(!ok(trace));
  finally_catch_all(trace);

  try(trace);
  throw_static(a, trace);
  assert(!catch_static(b, trace));
  throw_static(b, trace);
  assert(!catch_static(a, trace));
  throw_static(a, trace);
  assert(!catch_static(b, trace));
  assert(catch_static(a, trace));
  finally_catch_all(trace);
  assert(ok(trace));

  try(trace);
  try(trace);
  finally_rethrow_static(a, trace);
  finally_rethrow_static(b, trace);
  assert(ok(trace));

  try(trace);
  try(trace);
  throw_static(a, trace);
  finally_rethrow_static(b, trace);
  finally_rethrow_static(c, trace);
  assert(!catch_static(a, trace));
  assert(!catch_static(b, trace));
  assert(catch_static(c, trace));

  try(trace);
  throw_static(a, trace);
  try(trace);
  finally_rethrow_static(b, trace);
  finally_rethrow_static(c, trace);
  assert(!catch_static(a, trace));
  assert(!catch_static(b, trace));
  assert(catch_static(c, trace));

  throw_static(a, trace);
  try(trace);
  try(trace);
  finally_rethrow_static(b, trace);
  finally_rethrow_static(c, trace);
  assert(!catch_static(b, trace));
  assert(!catch_static(c, trace));
  assert(catch_static(a, trace));
}
