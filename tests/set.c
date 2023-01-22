#include "set.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void test_set() {
  struct trace *trace = create_trace();
  struct set *set = create_set(0, trace);
  assert(ok(trace));

  const char *s1 = "/home/nazar";

  assert(!is_in_set(s1, set));
  add_to_set(s1, set, trace);
  assert(ok(trace));
  assert(is_in_set(s1, set));

  const char *s2 = "keynumber1";

  assert(!is_in_set(s2, set));
  add_to_set(s2, set, trace);
  assert(ok(trace));
  assert(is_in_set(s2, set));
  assert(is_in_set(s1, set));

  const char *s3 = "yerkn11ke1" /* hash collision with s2 */;

  assert(!is_in_set(s3, set));
  add_to_set(s3, set, trace);
  assert(ok(trace));
  assert(is_in_set(s3, set));
  assert(is_in_set(s2, set));
  assert(is_in_set(s1, set));

  free_set(set);
  free(trace);
}
