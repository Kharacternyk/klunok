#include "config.h"
#include <assert.h>
#include <stdlib.h>

void check_default_config(struct config *config);

void test_config_static() {
  struct trace *trace = create_trace();
  struct config *config = load_config("", trace);
  assert(ok(trace));
  free(trace);
  check_default_config(config);
}
