#include "set.h"
#include "buffer.h"
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
  struct buffer_view *v1 = create_buffer_view(s1, trace);
  assert(ok(trace));

  assert(!is_in_set(v1, set));
  assert(get_count_in_set(v1, set) == 0);
  add_to_set(s1, set, trace);
  assert(ok(trace));
  assert(is_in_set(v1, set));

  const char *s2 = "my-data.txt";
  struct buffer_view *v2 = create_buffer_view(s2, trace);
  struct buffer_view *v3 = create_buffer_view(s2, trace);
  assert(ok(trace));

  assert(!is_in_set(v2, set));
  assert(!is_in_set(v3, set));
  add_to_set(s2, set, trace);
  assert(ok(trace));
  assert(is_in_set(v2, set));
  assert(is_in_set(v3, set));
  assert(is_in_set(v1, set));

  const char *s4 = "/";
  struct buffer_view *v4 = create_buffer_view(s4, trace);
  assert(ok(trace));

  assert(!is_in_set(v4, set));
  add_to_set(s4, set, trace);
  assert(ok(trace));
  assert(is_in_set(v4, set));
  assert(is_in_set(v2, set));
  assert(is_in_set(v1, set));

  remove_from_set(v1, set);
  assert(is_in_set(v4, set));
  assert(is_in_set(v2, set));
  assert(!is_in_set(v1, set));

  remove_from_set(v1, set);
  assert(is_in_set(v4, set));
  assert(is_in_set(v2, set));
  assert(!is_in_set(v1, set));

  assert(get_count_in_set(v4, set) == 1);
  add_to_set(s4, set, trace);
  assert(ok(trace));
  assert(get_count_in_set(v4, set) == 2);
  assert(is_in_set(v4, set));

  remove_from_set(v4, set);
  assert(ok(trace));
  assert(is_in_set(v4, set));

  remove_from_set(v4, set);
  assert(ok(trace));
  assert(!is_in_set(v4, set));

  free_buffer_view(v1);
  free_buffer_view(v2);
  free_buffer_view(v3);
  free_buffer_view(v4);

  free_set(set);
  free(trace);
}
