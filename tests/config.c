#include "config.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void typed_error_callback_function(const char **error_message) {
  perror(*error_message);
  assert(false);
}
void error_callback_function(void *error_message) {
  typed_error_callback_function(error_message);
}

int main() {
  const char *error_message = NULL;
  struct callback *error_callback =
      create_callback(error_callback_function, &error_message, NULL);
  struct config *config =
      load_config(TEST_ROOT "/config.lua", error_callback, &error_message);

  const struct set *editors = get_configured_editors(config);
  assert(is_in_set("vi", editors));
  assert(is_in_set("vim", editors));
  assert(is_in_set("nvim", editors));
  assert(is_in_set("rstudio", editors));
  assert(!is_in_set("qutebrowser", editors));

  const char *version_pattern = get_configured_version_pattern(config);
  assert(!strcmp(version_pattern, "v%Y"));
}
