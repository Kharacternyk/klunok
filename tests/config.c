#include "config.h"
#include <assert.h>
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
  assert(!is_in_set("qutebrowser", editors));

  const char *version_pattern = get_configured_version_pattern(config);
  assert(!strcmp(version_pattern, "v%Y-%m-%d-%H-%M"));

  free_config(config);

  config = load_config(TEST_ROOT "/configs/override.lua", &error_code,
                       &static_error_message, &dynamic_error_message);
  assert(!error_code);
  assert(!static_error_message);
  assert(!dynamic_error_message);

  version_pattern = get_configured_version_pattern(config);
  assert(!strcmp(version_pattern, "override"));

  free_config(config);
}
