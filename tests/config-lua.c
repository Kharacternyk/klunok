#include "config.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void check_default_config(struct config *config);

void test_config_lua() {
  struct trace *trace = create_trace();
  struct config *config = load_config(TEST_ROOT "/lua/empty.lua", trace);
  assert(ok(trace));

  check_default_config(config);

  config = load_config(TEST_ROOT "/lua/override.lua", trace);
  assert(ok(trace));

  const struct set *editors = get_configured_editors(config);
  assert(!is_in_set("vi", editors));
  assert(is_in_set("vim", editors));
  assert(is_in_set("nvim", editors));
  assert(is_in_set("rsession", editors));
  assert(is_in_set("cat", editors));

  const char *version_pattern = get_configured_version_pattern(config);
  assert(!strcmp(version_pattern, "override"));

  free_config(config);

  load_config(TEST_ROOT "/lua/broken-semantics.lua", trace);
  assert(!ok(trace));
  catch_all(trace);

  load_config(TEST_ROOT "/lua/broken-syntax.lua", trace);
  assert(!ok(trace));
  catch_all(trace);

  free(trace);
}
