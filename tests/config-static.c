#include "config.h"
#include "messages.h"
#include <assert.h>
#include <stdlib.h>

void check_default_config(struct config *config);

void test_config_static() {
  struct trace *trace = create_trace();
  struct config *config = load_config(NULL, trace);
  assert(ok(trace));
  check_default_config(config);
  config = load_config("", trace);
  assert(catch_static(messages.config.is_static, trace));
  free(trace);
}
