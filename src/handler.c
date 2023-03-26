#include "handler.h"
#include "bitmap.h"
#include "buffer.h"
#include "config.h"
#include "deref.h"
#include "elfinterp.h"
#include "extension.h"
#include "journal.h"
#include "linq.h"
#include "messages.h"
#include "set.h"
#include "store.h"
#include "timestamp.h"
#include "trace.h"
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

struct handler {
  char *config_path;
  struct config *config;
  struct journal *journal;
  struct linq *linq;
  struct bitmap *editor_pid_bitmap;
  struct set *elf_interpreters;
};

struct handler *load_handler(const char *config_path, struct trace *trace) {
  struct handler *handler = TNULL(calloc(1, sizeof(struct handler)), trace);
  if (!ok(trace)) {
    return NULL;
  }

  rethrow_check(trace);
  if (config_path) {
    handler->config_path = TNULL(realpath(config_path, NULL), trace);
  } else {
    handler->config_path = NULL;
  }
  handler->config = load_config(config_path, trace);
  if (config_path) {
    rethrow_context(config_path, trace);
  }
  rethrow_static(messages.handler.config.cannot_load, trace);

  if (ok(trace)) {
    handler->elf_interpreters =
        create_set(get_elf_interpreter_count_guess(handler->config), trace);
    handler->editor_pid_bitmap =
        create_bitmap(get_max_pid_guess(handler->config), trace);

    rethrow_check(trace);
    handler->linq = load_linq(get_queue_path(handler->config),
                              get_debounce_seconds(handler->config),
                              get_queue_size_guess(handler->config),
                              get_path_length_guess(handler->config), trace);
    rethrow_context(get_queue_path(handler->config), trace);
    rethrow_static(messages.handler.linq.cannot_load, trace);

    rethrow_check(trace);
    handler->journal =
        open_journal(get_journal_path(handler->config),
                     get_journal_timestamp_pattern(handler->config), trace);
    if (get_journal_path(handler->config)) {
      rethrow_context(get_journal_path(handler->config), trace);
    }
    rethrow_static(messages.handler.journal.cannot_open, trace);
  }

  if (!ok(trace)) {
    free_handler(handler);
    return NULL;
  }

  return handler;
}

void handle_open_exec(pid_t pid, int fd, struct handler *handler,
                      struct trace *trace) {
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

  if (is_in_set(exe_filename_view, get_editors(handler->config))) {
    event = get_event_open_exec_editor(handler->config);
    set_bit(pid, handler->editor_pid_bitmap, trace);

    char *interpreter = get_elf_interpreter(fd, trace);
    if (interpreter) {
      add_to_set(interpreter, handler->elf_interpreters, trace);
      free(interpreter);
    }
  } else if (get_bit(pid, handler->editor_pid_bitmap)) {
    struct buffer_view *file_path_view = create_buffer_view(file_path, trace);
    if (!is_in_set(file_path_view, handler->elf_interpreters)) {
      unset_bit(pid, handler->editor_pid_bitmap);
    }
    free_buffer_view(file_path_view);
  }

  rethrow_check(trace);
  write_to_journal(event, pid, file_path, handler->journal, trace);
  rethrow_context(get_journal_path(handler->config), trace);
  rethrow_static(messages.handler.journal.cannot_write_to, trace);

  free(file_path);
  free_buffer_view(exe_filename_view);
}

static bool should_push_to_linq(pid_t pid, const char *path,
                                struct handler *handler, struct trace *trace) {
  struct buffer *path_buffer = create_buffer(trace);
  concat_string(path, path_buffer, trace);
  const struct buffer_view *path_view = get_view(path_buffer);

  if (!ok(trace)) {
    free_buffer(path_buffer);
    return false;
  }
  if (is_in_set(path_view, get_history_paths(handler->config))) {
    free_buffer(path_buffer);
    return true;
  }
  if (!get_bit(pid, handler->editor_pid_bitmap)) {
    free_buffer(path_buffer);
    return false;
  }

  set_length(1, path_buffer);

  bool is_included = false;
  bool is_excluded = false;
  bool is_hidden = false;

  /*TODO period can be a separator. Also reverse lookup */
  for (const char *path_cursor = path; ok(trace); ++path_cursor) {
    char c = *path_cursor;
    if (!c || c == '/') {
      if (is_in_set(path_view, get_excluded_paths(handler->config))) {
        is_excluded = true;
        is_included = false;
      } else if (is_in_set(path_view, get_included_paths(handler->config))) {
        is_included = true;
        is_excluded = false;
      }
    } else if (c == '.' && !is_hidden) {
      is_hidden = *(path_cursor - 1) == '/';
    }
    if (!c) {
      break;
    }
  }

  free_buffer(path_buffer);

  return is_included || (!is_hidden && !is_excluded);
}

void handle_close_write(pid_t pid, int fd, struct handler *handler,
                        struct trace *trace) {
  char *file_path = deref_fd(fd, get_path_length_guess(handler->config), trace);
  if (!ok(trace)) {
    return;
  }

  const char *event = get_event_close_write_not_by_editor(handler->config);

  if (should_push_to_linq(pid, file_path, handler, trace)) {
    event = get_event_close_write_by_editor(handler->config);
    rethrow_check(trace);
    push_to_linq(file_path, handler->linq, trace);
    rethrow_context(file_path, trace);
    rethrow_static(messages.handler.linq.cannot_push, trace);
  }

  rethrow_check(trace);
  write_to_journal(event, pid, file_path, handler->journal, trace);
  rethrow_context(get_journal_path(handler->config), trace);
  rethrow_static(messages.handler.journal.cannot_write_to, trace);

  if (ok(trace) && handler->config_path &&
      !strcmp(file_path, handler->config_path)) {
    rethrow_check(trace);
    struct config *new_config = load_config(handler->config_path, trace);
    rethrow_context(handler->config_path, trace);
    rethrow_static(messages.handler.config.cannot_reload, trace);

    struct linq *new_linq = NULL;
    if (ok(trace) &&
        strcmp(get_queue_path(handler->config), get_queue_path(new_config))) {
      rethrow_check(trace);
      new_linq = load_linq(get_queue_path(new_config),
                           get_debounce_seconds(new_config),
                           get_queue_size_guess(new_config),
                           get_path_length_guess(new_config), trace);
      rethrow_context(get_queue_path(new_config), trace);
      rethrow_static(messages.handler.linq.cannot_reload, trace);
    }

    struct journal *new_journal = NULL;
    if (ok(trace)) {
      rethrow_check(trace);
      new_journal =
          open_journal(get_journal_path(new_config),
                       get_journal_timestamp_pattern(new_config), trace);
      rethrow_context(get_journal_path(new_config), trace);
      rethrow_static(messages.handler.journal.cannot_open, trace);
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
      redebounce_linq(get_debounce_seconds(new_config), handler->linq);
    }
  }

  free(file_path);
}

void handle_timeout(struct handler *handler, time_t *retry_after_seconds,
                    struct trace *trace) {
  for (;;) {
    rethrow_check(trace);
    char *path = get_linq_head(handler->linq, retry_after_seconds, trace);
    rethrow_static(messages.handler.linq.cannot_get_head, trace);
    if (*retry_after_seconds || !ok(trace)) {
      return;
    }

    char *base_version =
        get_timestamp(get_version_pattern(handler->config), NAME_MAX, trace);
    struct buffer *version_buffer = create_buffer(trace);
    concat_string(base_version, version_buffer, trace);
    free(base_version);
    if (!ok(trace)) {
      free(path);
      free_buffer(version_buffer);
      return;
    }
    if (strchr(get_string(get_view(version_buffer)), '/')) {
      throw_context(get_string(get_view(version_buffer)), trace);
      throw_static(messages.handler.version.has_slashes, trace);
      free(path);
      free_buffer(version_buffer);
      return;
    }

    const char *extension = get_file_extension(path);
    size_t version_base_length = get_length(get_view(version_buffer));
    size_t duplicate_count = 0;

    const char *event = get_event_queue_head_stored(handler->config);

    struct buffer_view *path_view = create_buffer_view(path, trace);

    for (;;) {
      concat_string(extension, version_buffer, trace);
      if (is_in_set(path_view, get_history_paths(handler->config))) {
        copy_delta_to_store(path, get_string(get_view(version_buffer)),
                            get_cursor_version(handler->config),
                            get_store_root(handler->config), trace);
      } else {
        copy_to_store(path, get_string(get_view(version_buffer)),
                      get_store_root(handler->config), trace);
      }
      if (catch_static(messages.store.copy.file_does_not_exist, trace)) {
        event = get_event_queue_head_deleted(handler->config);
      } else if (catch_static(messages.store.copy.permission_denied, trace)) {
        event = get_event_queue_head_forbidden(handler->config);
      }
      if (ok(trace)) {
        pop_from_linq(handler->linq, trace);
        break;
      } else if (catch_static(messages.store.copy.version_already_exists,
                              trace)) {
        ++duplicate_count;
        set_length(version_base_length, version_buffer);
        /*FIXME configure me*/
        concat_string("-", version_buffer, trace);
        concat_size(duplicate_count, version_buffer, trace);
      } else {
        throw_context(path, trace);
        throw_static(messages.handler.store.cannot_copy, trace);
        free(path);
        free_buffer(version_buffer);
        free_buffer_view(path_view);
        return;
      }
    }

    rethrow_check(trace);
    write_to_journal(event, 0, path, handler->journal, trace);
    rethrow_context(get_journal_path(handler->config), trace);
    rethrow_static(messages.handler.journal.cannot_write_to, trace);

    free(path);
    free_buffer(version_buffer);
    free_buffer_view(path_view);
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
