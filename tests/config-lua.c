#include "config.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void check_default_config(struct config *config);

int main() {
  struct trace *trace = create_trace();
  struct config *config = load_config(TEST_ROOT "/configs/empty.lua", trace);
  assert(!get_trace_message(trace));

  check_default_config(config);

  config = load_config(TEST_ROOT "/configs/override.lua", trace);
  assert(!get_trace_message(trace));

  const struct set *editors = get_configured_editors(config);
  assert(!is_in_set("vi", editors));
  assert(is_in_set("vim", editors));
  assert(is_in_set("nvim", editors));
  assert(is_in_set("rstudio", editors));
  assert(is_in_set("cat", editors));

  const char *version_pattern = get_configured_version_pattern(config);
  assert(!strcmp(version_pattern, "override"));

  free_config(config);

  load_config(TEST_ROOT "/configs/broken-semantics.lua", trace);
  assert(get_trace_message(trace));
  while (get_trace_message(trace)) {
    pop_trace_message(trace);
  }

  load_config(TEST_ROOT "/configs/broken-syntax.lua", trace);
  assert(get_trace_message(trace));
  while (get_trace_message(trace)) {
    pop_trace_message(trace);
  }

  free(trace);
}
