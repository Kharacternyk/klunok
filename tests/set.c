#include "set.h"
#include "messages.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void test_set() {
  struct trace *trace = create_trace();
  struct set *set = create_set(0, trace);
  assert(ok(trace));

  const char *s1 = "/home/nazar";

  assert(!is_in_set(s1, set));
  assert(get_count_in_set(s1, set) == 0);
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

  const char *s4 = "/";

  assert(!is_in_set(s4, set));
  add_to_set(s4, set, trace);
  assert(ok(trace));
  assert(is_in_set(s4, set));
  assert(is_in_set(s3, set));
  assert(is_in_set(s2, set));
  assert(is_in_set(s1, set));

  remove_from_set(s3, set);
  assert(is_in_set(s4, set));
  assert(!is_in_set(s3, set));
  assert(is_in_set(s2, set));
  assert(is_in_set(s1, set));

  remove_from_set(s1, set);
  assert(is_in_set(s4, set));
  assert(!is_in_set(s3, set));
  assert(is_in_set(s2, set));
  assert(!is_in_set(s1, set));

  remove_from_set(s1, set);
  assert(is_in_set(s4, set));
  assert(!is_in_set(s3, set));
  assert(is_in_set(s2, set));
  assert(!is_in_set(s1, set));

  assert(get_count_in_set(s4, set) == 1);
  add_to_set(s4, set, trace);
  assert(ok(trace));
  assert(get_count_in_set(s4, set) == 2);
  assert(is_in_set(s4, set));

  remove_from_set(s4, set);
  assert(ok(trace));
  assert(is_in_set(s4, set));

  remove_from_set(s4, set);
  assert(ok(trace));
  assert(!is_in_set(s4, set));

  set_count_in_set(5, s4, set, trace);
  assert(ok(trace));
  assert(is_in_set(s4, set));
  assert(get_count_in_set(s4, set) == 5);

  set_count_in_set(3, s4, set, trace);
  assert(ok(trace));
  assert(is_in_set(s4, set));
  assert(get_count_in_set(s4, set) == 3);

  set_count_in_set(0, s4, set, trace);
  assert(ok(trace));
  assert(!is_in_set(s4, set));
  assert(get_count_in_set(s4, set) == 0);

  free_set(set);
  free(trace);
}
