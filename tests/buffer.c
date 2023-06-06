#include "buffer.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define S1 "abc"
#define S2 " 1 2 3"
#define N 1230
#define SN "1230"

void test_buffer(struct trace *trace) {
  struct buffer *buffer = create_buffer(trace);
  assert(ok(trace));
  const struct buffer_view *view = get_view(buffer);
  assert(*get_string(view) == 0);
  assert(get_hash(view) == 0);

  concat_string(S1, buffer, trace);
  assert(ok(trace));
  assert(!strcmp(S1, get_string(view)));
  size_t hash = get_hash(view);

  concat_string(S2, buffer, trace);
  assert(ok(trace));
  assert(!strcmp(S1 S2, get_string(view)));
  assert(hash != get_hash(view));
  hash = get_hash(view);

  size_t saved_length = get_length(view);

  concat_size(N, buffer, trace);
  assert(ok(trace));
  assert(!strcmp(S1 S2 SN, get_string(view)));
  assert(hash != get_hash(view));

  set_length(saved_length, buffer);
  assert(!strcmp(S1 S2, get_string(view)));
  assert(hash == get_hash(view));

  concat_size(0, buffer, trace);
  assert(ok(trace));
  assert(!strcmp(S1 S2 "0", get_string(view)));
  assert(hash != get_hash(view));

  concat_bytes("ABCDEF", 4, buffer, trace);
  assert(ok(trace));
  assert(!strcmp(S1 S2 "0ABCD", get_string(view)));
  assert(hash != get_hash(view));

  free_buffer(buffer);
}
