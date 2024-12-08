#include "set.h"
#include "buffer.h"
#include "messages.h"
#include "trace.h"
#include <assert.h>

void test_set(struct trace *trace) {
  struct set *set = create_set(0, trace);
  assert(ok(trace));
  assert(is_empty(set));

  const char *s1 = "/home/nazar";
  struct buffer_view *v1 = create_buffer_view(s1, trace);
  assert(ok(trace));

  assert(!is_within(v1, set));
  assert(get_count(v1, set) == 0);
  add(s1, set, trace);
  assert(ok(trace));
  assert(is_within(v1, set));
  assert(!is_empty(set));

  const char *s2 = "my-data.txt";
  struct buffer_view *v2 = create_buffer_view(s2, trace);
  struct buffer_view *v3 = create_buffer_view(s2, trace);
  assert(ok(trace));

  assert(!is_within(v2, set));
  assert(!is_within(v3, set));
  add(s2, set, trace);
  assert(ok(trace));
  assert(is_within(v2, set));
  assert(is_within(v3, set));
  assert(is_within(v1, set));

  const char *s4 = "/";
  struct buffer_view *v4 = create_buffer_view(s4, trace);
  assert(ok(trace));

  assert(!is_within(v4, set));
  add(s4, set, trace);
  assert(ok(trace));
  assert(is_within(v4, set));
  assert(is_within(v2, set));
  assert(is_within(v1, set));

  pop(v1, set);
  assert(is_within(v4, set));
  assert(is_within(v2, set));
  assert(!is_within(v1, set));

  pop(v1, set);
  assert(is_within(v4, set));
  assert(is_within(v2, set));
  assert(!is_within(v1, set));

  assert(get_count(v4, set) == 1);
  add_with_metadata(s4, 11, set, trace);
  add_with_metadata(s4, 22, set, trace);
  assert(ok(trace));
  assert(get_count(v4, set) == 3);
  assert(get_last_metadata(v4, set) == 22);
  assert(is_within(v4, set));

  pop(v4, set);
  assert(ok(trace));
  assert(is_within(v4, set));

  pop(v4, set);
  pop(v4, set);
  assert(ok(trace));
  assert(!is_within(v4, set));
  assert(is_empty(set));

  free_buffer_view(v1);
  free_buffer_view(v2);
  free_buffer_view(v3);
  free_buffer_view(v4);

  free_set(set);
}
