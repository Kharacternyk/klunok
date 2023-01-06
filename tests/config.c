#include "config.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main() {
  int error_code = 0;
  char *error_message = NULL;
  bool is_generic_error = false;
  struct config *config =
      load_config(TEST_ROOT "/configs/empty.lua", &error_code, &error_message,
                  &is_generic_error);
  assert(!error_code);
  assert(!error_message);
  assert(!is_generic_error);

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
                       &error_message, &is_generic_error);
  assert(!error_code);
  assert(!error_message);
  assert(!is_generic_error);

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
              &error_message, &is_generic_error);
  assert(error_code || error_message || is_generic_error);

  free(error_message);
  error_code = 0;
  error_message = NULL;
  is_generic_error = false;

  load_config(TEST_ROOT "/configs/broken-syntax.lua", &error_code,
              &error_message, &is_generic_error);
  assert(error_code || error_message || is_generic_error);

  free(error_message);
}
