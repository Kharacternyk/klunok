#include "config.h"
#include "buffer.h"
#include "set.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void check_default_config(struct config *config) {
  struct trace *trace = create_trace();
  const struct set *editors = get_editors(config);
  struct buffer_view *cat = create_buffer_view("cat", trace);
  struct buffer_view *vi = create_buffer_view("vi", trace);
  assert(ok(trace));
  assert(!is_within(cat, editors));
  assert(is_within(vi, editors));
  free_buffer_view(cat);
  free_buffer_view(vi);
  free(trace);

  assert(is_empty(get_project_roots(config)));
  assert(is_empty(get_project_parents(config)));
  assert(is_empty(get_history_paths(config)));
  assert(is_empty(get_excluded_paths(config)));
  assert(is_empty(get_included_paths(config)));
  assert(is_empty(get_cluded_paths(config)));

  assert(!strcmp(get_store_root(config), "klunok/store"));
  assert(!strcmp(get_project_store_root(config), "klunok/projects"));
  assert(
      !strcmp(get_unstable_project_store_root(config), "klunok/var/projects"));
  assert(!strcmp(get_queue_path(config), "klunok/var/queue"));
  assert(!strcmp(get_journal_path(config), "klunok/var/journal"));
  assert(!strcmp(get_offset_store_root(config), "klunok/var/offsets"));
  assert(!strcmp(get_journal_timestamp_pattern(config), "%Y-%m-%d-%H-%M"));
  assert(!strcmp(get_version_pattern(config), "v%Y-%m-%d-%H-%M"));

  assert(get_debounce_seconds(config) == 60);
  assert(get_path_length_guess(config) == 1024);
  assert(get_max_pid_guess(config) == 1 << 15);
  assert(get_elf_interpreter_count_guess(config) == 1);
  assert(get_queue_size_guess(config) == get_debounce_seconds(config) * 2);

  assert(!get_event_open_exec_not_editor(config));
  assert(!get_event_open_exec_editor(config));
  assert(!get_event_close_write_not_by_editor(config));
  assert(!get_event_close_write_by_editor(config));
  assert(!get_event_queue_head_deleted(config));
  assert(!get_event_queue_head_forbidden(config));
  assert(!strcmp(get_event_queue_head_stored(config), ""));

  free_config(config);
}
