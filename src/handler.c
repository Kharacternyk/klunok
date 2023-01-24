#include "handler.h"
#include "bitmap.h"
#include "builder.h"
#include "deref.h"
#include "elfinterp.h"
#include "extension.h"
#include "linq.h"
#include "messages.h"
#include "store.h"
#include "timestamp.h"
#include <assert.h>
#include <errno.h>
#include <fnmatch.h>
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
  handler->config_path = TNULL(strdup(config_path), trace);
  if (!ok(trace)) {
    free(handler);
    return NULL;
  }

  handler->config = load_config(config_path, trace);
  if (!ok(trace)) {
    throw_static(messages.handler.config.cannot_load, trace);
    free(handler);
    return NULL;
  }

  handler->elf_interpreters = create_set(
      get_configured_elf_interpreter_count_guess(handler->config), trace);
  handler->handled_executables =
      create_set(get_configured_executable_count_guess(handler->config), trace);
  handler->editor_pid_bitmap =
      create_bitmap(get_configured_max_pid_guess(handler->config), trace);
  if (!ok(trace)) {
    free(handler);
    return NULL;
  }

  handler->linq =
      load_linq(get_configured_queue_path(handler->config),
                get_configured_debounce_seconds(handler->config),
                get_configured_queue_size_guess(handler->config),
                get_configured_path_length_guess(handler->config), trace);
  if (!ok(trace)) {
    throw_static(messages.handler.linq.cannot_load, trace);
    free(handler);
    return NULL;
  }

  return handler;
}

void handle_open_exec(pid_t pid, int fd, struct handler *handler,
                      struct trace *trace) {
  char *file_path =
      deref_fd(fd, get_configured_path_length_guess(handler->config), trace);
  if (!ok(trace)) {
    return;
  }
  char *exe_filename = strrchr(file_path, '/');
  assert(exe_filename);
  ++exe_filename;

  if (!is_in_set(file_path, handler->elf_interpreters)) {
    if (is_in_set(exe_filename, get_configured_editors(handler->config))) {
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
  char *file_path =
      deref_fd(fd, get_configured_path_length_guess(handler->config), trace);
  if (!ok(trace)) {
    return;
  }
  if (get_bit_in_bitmap(pid, handler->editor_pid_bitmap) &&
      /*FIXME*/ strstr(file_path, "/.") == NULL) {
    push_to_linq(file_path, handler->linq, trace);
    if (!ok(trace)) {
      throw_static(messages.handler.linq.cannot_push, trace);
      return free(file_path);
    }
  }

  if (!strcmp(file_path, handler->config_path)) {
    struct config *new_config = load_config(handler->config_path, trace);
    if (!ok(trace)) {
      throw_static(messages.handler.config.cannot_reload, trace);
      return free(file_path);
    }

    if (strcmp(get_configured_queue_path(handler->config),
               get_configured_queue_path(new_config))) {
      struct linq *new_linq =
          load_linq(get_configured_queue_path(new_config),
                    get_configured_debounce_seconds(new_config),
                    get_configured_queue_size_guess(new_config),
                    get_configured_path_length_guess(new_config), trace);
      if (!ok(trace)) {
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
  *retry_after_seconds = 0;
  for (;;) {
    char *path = get_linq_head(handler->linq, retry_after_seconds, trace);
    if (!ok(trace)) {
      throw_static(messages.handler.linq.cannot_get_head, trace);
      return;
    }
    if (*retry_after_seconds) {
      return;
    }

    char *base_version = get_timestamp(
        get_configured_version_pattern(handler->config),
        get_configured_version_max_length(handler->config), trace);
    struct builder *version_builder = create_builder(trace);
    concat_string(base_version, version_builder, trace);
    free(base_version);
    if (!ok(trace)) {
      free(path);
      free_builder(version_builder);
      return;
    }
    if (strchr(build_string(version_builder), '/')) {
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
                    get_configured_store_root(handler->config), trace);
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
