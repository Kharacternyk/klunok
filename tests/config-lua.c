#include "buffer.h"
#include "config.h"
#include "set.h"
#include "test-constants.h"
#include "trace.h"
#include <assert.h>
#include <string.h>

void check_default_config(struct config *config);

void test_config_lua(struct trace *trace) {
  struct config *config = load_config(NULL, trace);
  assert(ok(trace));
  check_default_config(config);

  config = load_config(TEST_ROOT "/lua/empty.lua", trace);
  assert(ok(trace));
  check_default_config(config);

  config = load_config(TEST_ROOT "/lua/override.lua", trace);
  assert(ok(trace));

  const struct set *editors = get_editors(config);
  const struct set *ignored_leading_dots = get_ignored_leading_dots(config);
  struct buffer_view *cat = create_buffer_view("cat", trace);
  struct buffer_view *vi = create_buffer_view("vi", trace);
  struct buffer_view *custom_hidden =
      create_buffer_view(".custom-hidden", trace);
  struct buffer_view *env = create_buffer_view(".env", trace);
  assert(ok(trace));
  assert(is_within(cat, editors));
  assert(!is_within(vi, editors));
  assert(is_within(custom_hidden, ignored_leading_dots));
  assert(!is_within(env, ignored_leading_dots));
  free_buffer_view(cat);
  free_buffer_view(vi);
  free_buffer_view(custom_hidden);
  free_buffer_view(env);

  const char *version_pattern = get_version_pattern(config);
  assert(!strcmp(version_pattern, "override"));

  free_config(config);

  try(trace);
  config = load_config(TEST_ROOT "/lua/broken-semantics.lua", trace);
  assert(!ok(trace));
  finally_catch_all(trace);

  try(trace);
  config = load_config(TEST_ROOT "/lua/broken-syntax.lua", trace);
  assert(!ok(trace));
  finally_catch_all(trace);
}
