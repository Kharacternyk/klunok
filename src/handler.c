#include "handler.h"
#include "bitmap.h"
#include "builder.h"
#include "config.h"
#include "deref.h"
#include "elfinterp.h"
#include "extension.h"
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
  struct linq *linq;
  struct bitmap *editor_pid_bitmap;
  struct set *elf_interpreters;
  struct set *handled_executables;
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
    handler->handled_executables =
        create_set(get_executable_count_guess(handler->config), trace);
    handler->editor_pid_bitmap =
        create_bitmap(get_max_pid_guess(handler->config), trace);

    rethrow_check(trace);
    handler->linq = load_linq(get_queue_path(handler->config),
                              get_debounce_seconds(handler->config),
                              get_queue_size_guess(handler->config),
                              get_path_length_guess(handler->config), trace);
    rethrow_context(get_queue_path(handler->config), trace);
    rethrow_static(messages.handler.linq.cannot_load, trace);
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

  if (!is_in_set(file_path, handler->elf_interpreters)) {
    if (is_in_set(exe_filename, get_editors(handler->config))) {
      set_bit_in_bitmap(pid, handler->editor_pid_bitmap, trace);
    } else {
      unset_bit_in_bitmap(pid, handler->editor_pid_bitmap);
    }

    if (!is_in_set(file_path, handler->handled_executables)) {
      char *interpreter = get_elf_interpreter(fd, trace);
      if (ok(trace) && interpreter) {
        add_to_set(interpreter, handler->elf_interpreters, trace);
        free(interpreter);
      }
    }
  }
  free(file_path);
}

void handle_close_write(pid_t pid, int fd, struct handler *handler,
                        struct trace *trace) {
  char *file_path = deref_fd(fd, get_path_length_guess(handler->config), trace);
  if (!ok(trace)) {
    return;
  }
  if (get_bit_in_bitmap(pid, handler->editor_pid_bitmap) &&
      /*FIXME*/ strstr(file_path, "/.") == NULL) {
    push_to_linq(file_path, handler->linq, trace);
    if (!ok(trace)) {
      throw_context(file_path, trace);
      throw_static(messages.handler.linq.cannot_push, trace);
      return free(file_path);
    }
  }

  if (handler->config_path && !strcmp(file_path, handler->config_path)) {
    struct config *new_config = load_config(handler->config_path, trace);
    if (!ok(trace)) {
      throw_context(handler->config_path, trace);
      throw_static(messages.handler.config.cannot_reload, trace);
      return free(file_path);
    }

    if (strcmp(get_queue_path(handler->config), get_queue_path(new_config))) {
      struct linq *new_linq = load_linq(
          get_queue_path(new_config), get_debounce_seconds(new_config),
          get_queue_size_guess(new_config), get_path_length_guess(new_config),
          trace);
      if (!ok(trace)) {
        throw_context(get_queue_path(new_config), trace);
        throw_static(messages.handler.linq.cannot_reload, trace);
        free_config(new_config);
        return free(file_path);
      }
      free_linq(handler->linq);
      handler->linq = new_linq;
    }

    free_config(handler->config);
    handler->config = new_config;
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
    struct builder *version_builder = create_builder(trace);
    concat_string(base_version, version_builder, trace);
    free(base_version);
    if (!ok(trace)) {
      free(path);
      free_builder(version_builder);
      return;
    }
    if (strchr(build_string(version_builder), '/')) {
      throw_context(build_string(version_builder), trace);
      throw_static(messages.handler.version.has_slashes, trace);
      free(path);
      free_builder(version_builder);
      return;
    }

    const char *extension = get_file_extension(path);
    size_t version_base_length = get_builder_length(version_builder);
    size_t duplicate_count = 0;

    for (;;) {
      concat_string(extension, version_builder, trace);
      copy_to_store(path, build_string(version_builder),
                    get_store_root(handler->config), trace);
      catch_static(messages.store.copy.file_does_not_exist, trace);
      catch_static(messages.store.copy.permission_denied, trace);
      if (ok(trace)) {
        pop_from_linq(handler->linq, trace);
        break;
      } else if (catch_static(messages.store.copy.version_already_exists,
                              trace)) {
        ++duplicate_count;
        truncate_builder(version_base_length, version_builder);
        /*FIXME configure me*/
        concat_string("-", version_builder, trace);
        concat_size(duplicate_count, version_builder, trace);
      } else {
        throw_context(path, trace);
        throw_static(messages.handler.store.cannot_copy, trace);
        free(path);
        free_builder(version_builder);
        return;
      }
    }

    free(path);
    free_builder(version_builder);
  }
}

void free_handler(struct handler *handler) {
  if (handler) {
    free(handler->config_path);
    free_config(handler->config);
    free_linq(handler->linq);
    free_bitmap(handler->editor_pid_bitmap);
    free_set(handler->elf_interpreters);
    free_set(handler->handled_executables);
    free(handler);
  }
}
