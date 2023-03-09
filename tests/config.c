#include "config.h"
#include "set.h"
#include <assert.h>
#include <string.h>

void check_default_config(struct config *config) {
  assert(0 != path_excluded);
  assert(path_included != path_excluded);

  const struct set *editors = get_editors(config);
  assert(is_in_set("vi", editors));
  assert(is_in_set("vim", editors));
  assert(is_in_set("nvim", editors));
  assert(is_in_set("rsession", editors));
  assert(!is_in_set("cat", editors));

  assert(!strcmp(get_store_root(config), "./klunok/store"));
  assert(!strcmp(get_queue_path(config), "./klunok/var/queue"));
  assert(!strcmp(get_journal_path(config), "./klunok/var/journal"));
  assert(!strcmp(get_journal_timestamp_pattern(config), "%Y-%m-%d-%H-%M"));
  assert(!strcmp(get_version_pattern(config), "v%Y-%m-%d-%H-%M"));
  assert(!strcmp(get_cursor_version(config), ".cursor"));

  assert(get_debounce_seconds(config) == 60);
  assert(get_path_length_guess(config) == 1024);
  assert(get_max_pid_guess(config) == 1 << 15);
  assert(get_elf_interpreter_count_guess(config) == 1);
  assert(get_executable_count_guess(config) == 128);
  assert(get_queue_size_guess(config) == get_debounce_seconds(config) * 2);

  assert(!get_event_open_exec_not_editor(config));
  assert(!get_event_open_exec_editor(config));
  assert(!get_event_open_exec_interpreter(config));
  assert(!get_event_close_write_not_by_editor(config));
  assert(!get_event_close_write_by_editor(config));
  assert(!get_event_queue_head_deleted(config));
  assert(!get_event_queue_head_forbidden(config));
  assert(!strcmp(get_event_queue_head_stored(config), ""));

  free_config(config);
}
