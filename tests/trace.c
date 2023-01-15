#include "trace.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int main() {
  struct trace *trace = create_trace();
  assert(trace);
  assert(!get_trace_message(trace));

  const char *a = "abc";
  const char *b = "XYZ";

  trace_static(a, trace);
  trace_dynamic(b, trace);
  errno = ENOMEM;
  trace_errno(trace);

  assert(get_trace_message(trace));
  assert(!strcmp(get_trace_message(trace), strerror(ENOMEM)));
  pop_trace_message(trace);
  assert(!strcmp(get_trace_message(trace), b));
  assert(get_trace_message(trace) != b);
  pop_trace_message(trace);
  assert(!strcmp(get_trace_message(trace), a));
  assert(get_trace_message(trace) == a);
  pop_trace_message(trace);
  assert(!get_trace_message(trace));

  free(trace);
}
