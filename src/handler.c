#include "handler.h"
#include "bitmap.h"
#include "buffer.h"
#include "config.h"
#include "copy.h"
#include "counter.h"
#include "deref.h"
#include "elfinterp.h"
#include "extension.h"
#include "journal.h"
#include "linq.h"
#include "messages.h"
#include "set.h"
#include "sieve.h"
#include "timestamp.h"
#include "trace.h"
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

struct handler {
  char *config_path;
  size_t common_parent_path_length;
  struct config *config;
  struct journal *journal;
  struct linq *linq;
  struct bitmap *editor_pid_bitmap;
  struct set *elf_interpreters;
};

struct handler *load_handler(const char *config_path,
                             size_t common_parent_path_length,
                             struct trace *trace) {
  struct handler *handler = TNULL(calloc(1, sizeof(struct handler)), trace);
  if (!ok(trace)) {
    return NULL;
  }

  handler->common_parent_path_length = common_parent_path_length;

  try(trace);
  if (config_path) {
    handler->config_path = TNULL(realpath(config_path, NULL), trace);
  } else {
    handler->config_path = NULL;
  }
  handler->config = load_config(config_path, trace);
  if (config_path) {
    rethrow_context(config_path, trace);
  }
  finally_rethrow_static(messages.handler.config.cannot_load, trace);

  if (ok(trace)) {
    handler->elf_interpreters =
        create_set(get_elf_interpreter_count_guess(handler->config), trace);
    handler->editor_pid_bitmap =
        create_bitmap(get_max_pid_guess(handler->config), trace);

    try(trace);
    handler->linq = load_linq(get_queue_path(handler->config),
                              get_debounce_seconds(handler->config),
                              get_queue_size_guess(handler->config),
                              get_path_length_guess(handler->config), trace);
    rethrow_context(get_queue_path(handler->config), trace);
    finally_rethrow_static(messages.handler.linq.cannot_load, trace);

    try(trace);
    handler->journal =
        open_journal(get_journal_path(handler->config),
                     get_journal_timestamp_pattern(handler->config), trace);
    if (get_journal_path(handler->config)) {
      rethrow_context(get_journal_path(handler->config), trace);
    }
    finally_rethrow_static(messages.handler.journal.cannot_open, trace);
  }

  if (!ok(trace)) {
    free_handler(handler);
    return NULL;
  }

  return handler;
}

void handle_open_exec(pid_t pid, int fd, struct handler *handler,
                      struct trace *trace) {
  if (!ok(trace)) {
    return;
  }
  char *file_path = deref_fd(fd, get_path_length_guess(handler->config), trace);
  if (!ok(trace)) {
    return;
  }
  char *exe_filename = strrchr(file_path, '/');
  assert(exe_filename);
  ++exe_filename;

  const char *event = get_event_open_exec_not_editor(handler->config);

  struct buffer_view *exe_filename_view =
      create_buffer_view(exe_filename, trace);

  if (!ok(trace)) {
    free_buffer_view(exe_filename_view);
    return;
  }

  if (is_within(exe_filename_view, get_editors(handler->config))) {
    event = get_event_open_exec_editor(handler->config);
    set_bit(pid, handler->editor_pid_bitmap, trace);

    char *interpreter = get_elf_interpreter(fd, trace);
    if (interpreter) {
      add(interpreter, handler->elf_interpreters, trace);
      free(interpreter);
    }
  } else if (get_bit(pid, handler->editor_pid_bitmap)) {
    struct buffer_view *file_path_view = create_buffer_view(file_path, trace);
    if (ok(trace) && !is_within(file_path_view, handler->elf_interpreters)) {
      unset_bit(pid, handler->editor_pid_bitmap);
    }
    free_buffer_view(file_path_view);
  }

  try(trace);
  note(event, pid, file_path, handler->journal, trace);
  rethrow_context(get_journal_path(handler->config), trace);
  finally_rethrow_static(messages.handler.journal.cannot_write_to, trace);

  free(file_path);
  free_buffer_view(exe_filename_view);
}

static bool should_push_to_linq(pid_t pid, const char *path,
                                struct handler *handler, struct trace *trace) {
  if (!ok(trace)) {
    return false;
  }
  const struct set *sets[] = {
      get_included_paths(handler->config),
      get_excluded_paths(handler->config),
      get_history_paths(handler->config),
  };
  struct sieved_path *sieved_path =
      sieve(path, handler->common_parent_path_length, sets, 3, trace);

  if (!ok(trace)) {
    free_sieved_path(sieved_path);
    return false;
  }

  const char *const *ends = get_sieved_ends(sieved_path);
  const char *included_end = ends[0];
  const char *history_end = ends[2];
  const char *excluded_end = ends[1] > get_hiding_dot(sieved_path)
                                 ? ends[1]
                                 : get_hiding_dot(sieved_path);

  free_sieved_path(sieved_path);

  return history_end || (get_bit(pid, handler->editor_pid_bitmap) &&
                         included_end >= excluded_end);
}

void handle_close_write(pid_t pid, int fd, struct handler *handler,
                        struct trace *trace) {
  if (!ok(trace)) {
    return;
  }
  char *file_path = deref_fd(fd, get_path_length_guess(handler->config), trace);
  if (!ok(trace)) {
    return;
  }

  const char *event = get_event_close_write_not_by_editor(handler->config);

  if (should_push_to_linq(pid, file_path, handler, trace)) {
    event = get_event_close_write_by_editor(handler->config);
    try(trace);
    push(file_path, handler->linq, trace);
    rethrow_context(file_path, trace);
    finally_rethrow_static(messages.handler.linq.cannot_push, trace);
  }

  try(trace);
  note(event, pid, file_path, handler->journal, trace);
  rethrow_context(get_journal_path(handler->config), trace);
  finally_rethrow_static(messages.handler.journal.cannot_write_to, trace);

  if (ok(trace) && handler->config_path &&
      !strcmp(file_path, handler->config_path)) {
    try(trace);
    struct config *new_config = load_config(handler->config_path, trace);
    rethrow_context(handler->config_path, trace);
    finally_rethrow_static(messages.handler.config.cannot_reload, trace);

    struct linq *new_linq = NULL;
    if (ok(trace) &&
        strcmp(get_queue_path(handler->config), get_queue_path(new_config))) {
      try(trace);
      new_linq = load_linq(get_queue_path(new_config),
                           get_debounce_seconds(new_config),
                           get_queue_size_guess(new_config),
                           get_path_length_guess(new_config), trace);
      rethrow_context(get_queue_path(new_config), trace);
      finally_rethrow_static(messages.handler.linq.cannot_reload, trace);
    }

    struct journal *new_journal = NULL;
    if (ok(trace)) {
      try(trace);
      new_journal =
          open_journal(get_journal_path(new_config),
                       get_journal_timestamp_pattern(new_config), trace);
      rethrow_context(get_journal_path(new_config), trace);
      finally_rethrow_static(messages.handler.journal.cannot_open, trace);
    }

    if (ok(trace)) {
      free_config(handler->config);
      handler->config = new_config;
      free_journal(handler->journal);
      handler->journal = new_journal;
      if (new_linq) {
        free_linq(handler->linq);
        handler->linq = new_linq;
      }
      redebounce(get_debounce_seconds(new_config), handler->linq);
    }
  }

  free(file_path);
}

void handle_timeout(struct handler *handler, time_t *retry_after_seconds,
                    struct trace *trace) {
  while (ok(trace)) {
    try(trace);
    char *absolute_path = get_head(handler->linq, retry_after_seconds, trace);
    finally_rethrow_static(messages.handler.linq.cannot_get_head, trace);
    if (*retry_after_seconds || !ok(trace)) {
      return;
    }

    const char *relative_path =
        absolute_path + handler->common_parent_path_length;

    char *base_version =
        get_timestamp(get_version_pattern(handler->config), NAME_MAX, trace);
    if (strchr(base_version, '/')) {
      throw_context(base_version, trace);
      throw_static(messages.handler.version.has_slashes, trace);
      free(absolute_path);
      free(base_version);
      return;
    }

    struct buffer *store_path = create_buffer(trace);
    concat_string(get_store_root(handler->config), store_path, trace);
    concat_char('/', store_path, trace);
    concat_string(relative_path, store_path, trace);
    concat_char('/', store_path, trace);
    concat_string(base_version, store_path, trace);
    free(base_version);

    struct buffer *offset_path = create_buffer(trace);
    concat_string(get_offset_store_root(handler->config), offset_path, trace);
    concat_char('/', offset_path, trace);
    concat_string(relative_path, offset_path, trace);

    bool is_history_path = false;
    size_t offset = 0;
    struct buffer_view *absolute_path_view =
        create_buffer_view(absolute_path, trace);
    struct buffer_view *relative_path_view =
        create_buffer_view(relative_path, trace);

    if (ok(trace) &&
        (is_within(absolute_path_view, get_history_paths(handler->config)) ||
         is_within(relative_path_view, get_history_paths(handler->config)))) {
      is_history_path = true;
      offset = read_counter(get_string(get_view(offset_path)), trace);
    }

    free_buffer_view(absolute_path_view);
    free_buffer_view(relative_path_view);

    if (!ok(trace)) {
      free(absolute_path);
      free_buffer(store_path);
      free_buffer(offset_path);
      return;
    }

    const char *extension = get_file_extension(relative_path);
    size_t store_path_base_length = get_length(get_view(store_path));
    size_t duplicate_count = 0;

    const char *event = get_event_queue_head_stored(handler->config);

    for (;;) {
      concat_string(extension, store_path, trace);

      try(trace);
      size_t new_offset = copy_file(get_string(get_view(store_path)),
                                    absolute_path, offset, trace);
      if (!is_history_path) {
        new_offset = 0;
      }
      if (catch_static(messages.copy.source_does_not_exist, trace)) {
        event = get_event_queue_head_deleted(handler->config);
      } else if (catch_static(messages.copy.source_permission_denied, trace)) {
        event = get_event_queue_head_forbidden(handler->config);
      } else if (catch_static(messages.copy.destination_already_exists,
                              trace)) {
        ++duplicate_count;
        set_length(store_path_base_length, store_path);
        /*FIXME configure me*/
        concat_string("-", store_path, trace);
        concat_size(duplicate_count, store_path, trace);
        continue;
      } else {
        write_counter(get_string(get_view(offset_path)), new_offset, trace);
      }
      finally(trace);

      if (ok(trace)) {
        pop_head(handler->linq, trace);
        break;
      } else {
        throw_context(absolute_path, trace);
        throw_static(messages.handler.store.cannot_copy, trace);
        free(absolute_path);
        free_buffer(store_path);
        free_buffer(offset_path);
        return;
      }
    }

    try(trace);
    note(event, 0, relative_path, handler->journal, trace);
    rethrow_context(get_journal_path(handler->config), trace);
    finally_rethrow_static(messages.handler.journal.cannot_write_to, trace);

    free(absolute_path);
    free_buffer(store_path);
    free_buffer(offset_path);
  }
}

void free_handler(struct handler *handler) {
  if (handler) {
    free(handler->config_path);
    free_config(handler->config);
    free_journal(handler->journal);
    free_linq(handler->linq);
    free_bitmap(handler->editor_pid_bitmap);
    free_set(handler->elf_interpreters);
    free(handler);
  }
}
