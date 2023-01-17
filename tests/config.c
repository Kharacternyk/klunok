#include "config.h"
#include <assert.h>
#include <string.h>

void check_default_config(struct config *config) {
  const struct set *editors = get_configured_editors(config);
  assert(is_in_set("vi", editors));
  assert(is_in_set("vim", editors));
  assert(is_in_set("nvim", editors));
  assert(is_in_set("rsession", editors));
  assert(!is_in_set("cat", editors));

  assert(!strcmp(get_configured_store_root(config), "./klunok/store"));
  assert(!strcmp(get_configured_queue_path(config), "./klunok/queue"));
  assert(!strcmp(get_configured_version_pattern(config), "v%Y-%m-%d-%H-%M"));

  assert(get_configured_debounce_seconds(config) == 60);
  assert(get_configured_version_max_length(config) == 80);
  assert(get_configured_path_length_guess(config) == 1024);
  assert(get_configured_max_pid_guess(config) == 1 << 16);

  free_config(config);
}
