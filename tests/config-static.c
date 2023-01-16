#include "config.h"
#include <assert.h>
#include <stdlib.h>

void check_default_config(struct config *config);

int main() {
  struct trace *trace = create_trace();
  struct config *config = load_config("", trace);
  assert(!get_trace_message(trace));
  free(trace);
  check_default_config(config);
}
