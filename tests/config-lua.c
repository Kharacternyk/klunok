#include "buffer.h"
#include "config.h"
#include "set.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void check_default_config(struct config *config);

void test_config_lua() {
  struct trace *trace = create_trace();
  struct config *config = load_config(NULL, trace);
  assert(ok(trace));
  check_default_config(config);

  config = load_config(TEST_ROOT "/lua/empty.lua", trace);
  assert(ok(trace));
  check_default_config(config);

  config = load_config(TEST_ROOT "/lua/override.lua", trace);
  assert(ok(trace));

  const struct set *editors = get_editors(config);
  struct buffer_view *cat = create_buffer_view("cat", trace);
  struct buffer_view *vi = create_buffer_view("vi", trace);
  assert(ok(trace));
  assert(is_in_set(cat, editors));
  assert(!is_in_set(vi, editors));
  free_buffer_view(cat);
  free_buffer_view(vi);

  const char *version_pattern = get_version_pattern(config);
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
