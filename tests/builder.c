#include "builder.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define S1 "abc"
#define S2 " 1 2 3"

int main() {
  struct trace *trace = create_trace();
  struct builder *builder = create_builder(trace);
  assert(ok(trace));
  assert(*build_string(builder) == 0);

  append_to_builder(S1, builder, trace);
  assert(ok(trace));
  assert(!strcmp(S1, build_string(builder)));

  append_to_builder(S2, builder, trace);
  assert(ok(trace));
  assert(!strcmp(S1 S2, build_string(builder)));

  free_builder(builder);
  free(trace);
}
