#include "config.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main() {
  int error_code = 0;
  const char *static_error_message = NULL;
  char *dynamic_error_message = NULL;
  struct config *config =
      load_config(TEST_ROOT "/configs/empty.lua", &error_code,
                  &static_error_message, &dynamic_error_message);
  assert(!error_code);
  assert(!static_error_message);
  assert(!dynamic_error_message);

  const struct set *editors = get_configured_editors(config);
  assert(is_in_set("vi", editors));
  assert(is_in_set("vim", editors));
  assert(is_in_set("nvim", editors));
  assert(is_in_set("rstudio", editors));
  assert(!is_in_set("cat", editors));

  const char *version_pattern = get_configured_version_pattern(config);
  assert(!strcmp(version_pattern, "v%Y-%m-%d-%H-%M"));

  free_config(config);

  config = load_config(TEST_ROOT "/configs/override.lua", &error_code,
                       &static_error_message, &dynamic_error_message);
  assert(!error_code);
  assert(!static_error_message);
  assert(!dynamic_error_message);

  editors = get_configured_editors(config);
  assert(!is_in_set("vi", editors));
  assert(is_in_set("vim", editors));
  assert(is_in_set("nvim", editors));
  assert(is_in_set("rstudio", editors));
  assert(is_in_set("cat", editors));

  version_pattern = get_configured_version_pattern(config);
  assert(!strcmp(version_pattern, "override"));

  free_config(config);

  load_config(TEST_ROOT "/configs/broken-semantics.lua", &error_code,
              &static_error_message, &dynamic_error_message);
  assert(error_code || dynamic_error_message || static_error_message);

  free(dynamic_error_message);
  error_code = 0;
  dynamic_error_message = NULL;
  static_error_message = NULL;

  load_config(TEST_ROOT "/configs/broken-syntax.lua", &error_code,
              &static_error_message, &dynamic_error_message);
  assert(error_code || dynamic_error_message || static_error_message);

  free(dynamic_error_message);
}
