#include "builder.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define S1 "abc"
#define S2 " 1 2 3"
#define N 1230
#define SN "1230"

void test_builder() {
  struct trace *trace = create_trace();
  struct builder *builder = create_builder(trace);
  assert(ok(trace));
  assert(*get_string(builder) == 0);

  concat_string(S1, builder, trace);
  assert(ok(trace));
  assert(!strcmp(S1, get_string(builder)));

  concat_string(S2, builder, trace);
  assert(ok(trace));
  assert(!strcmp(S1 S2, get_string(builder)));

  size_t saved_length = get_builder_length(builder);

  concat_size(N, builder, trace);
  assert(ok(trace));
  assert(!strcmp(S1 S2 SN, get_string(builder)));

  truncate_builder(saved_length, builder);
  assert(!strcmp(S1 S2, get_string(builder)));

  concat_size(0, builder, trace);
  assert(ok(trace));
  assert(!strcmp(S1 S2 "0", get_string(builder)));

  free_builder(builder);
  free(trace);
}
