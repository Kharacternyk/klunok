#include "parents.h"
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#define DIRECTORY TEST_ROOT "/parents/abc/defgh/123"

int main() {
  struct trace *trace = create_trace();

  rmdir(DIRECTORY);
  assert(access(DIRECTORY, F_OK) != 0);
  create_parents(DIRECTORY "/file", S_IRWXU, trace);
  assert(!get_trace_message(trace));
  assert(access(DIRECTORY, F_OK) == 0);
  remove_empty_parents(DIRECTORY "/file", trace);
  assert(!get_trace_message(trace));
  assert(access(DIRECTORY, F_OK) != 0);

  free(trace);
}
