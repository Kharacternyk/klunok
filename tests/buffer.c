#include "buffer.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define S1 "abc"
#define S2 " 1 2 3"
#define N 1230
#define SN "1230"

void test_buffer() {
  struct trace *trace = create_trace();
  struct buffer *buffer = create_buffer(trace);
  assert(ok(trace));
  assert(*get_string(buffer) == 0);
  assert(get_hash(buffer) == 0);

  concat_string(S1, buffer, trace);
  assert(ok(trace));
  assert(!strcmp(S1, get_string(buffer)));
  size_t hash = get_hash(buffer);

  concat_string(S2, buffer, trace);
  assert(ok(trace));
  assert(!strcmp(S1 S2, get_string(buffer)));
  assert(hash != get_hash(buffer));
  hash = get_hash(buffer);

  size_t saved_length = get_length(buffer);

  concat_size(N, buffer, trace);
  assert(ok(trace));
  assert(!strcmp(S1 S2 SN, get_string(buffer)));
  assert(hash != get_hash(buffer));

  set_length(saved_length, buffer);
  assert(!strcmp(S1 S2, get_string(buffer)));
  assert(hash == get_hash(buffer));

  concat_size(0, buffer, trace);
  assert(ok(trace));
  assert(!strcmp(S1 S2 "0", get_string(buffer)));
  assert(hash != get_hash(buffer));

  free_buffer(buffer);
  free(trace);
}
