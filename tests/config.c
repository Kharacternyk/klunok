#include "config.h"
#include <assert.h>
#include <string.h>

int main() {
  int error_code = 0;
  const char *static_error_message = NULL;
  char *dynamic_error_message = NULL;
  struct config *config =
      load_config(TEST_ROOT "/config.lua", &error_code, &static_error_message,
                  &dynamic_error_message);
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
  assert(!strcmp(version_pattern, "v%Y"));

  free_config(config);
}
