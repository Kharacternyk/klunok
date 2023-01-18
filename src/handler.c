#include "handler.h"
#include "bitmap.h"
#include "deref.h"
#include "elfinterp.h"
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
  struct store *store;
  struct bitmap *editor_pid_bitmap;
  struct set *elf_interpeters;
  struct set *handled_executables;
};

struct handler *load_handler(const char *config_path, struct trace *trace) {
  struct handler *handler = calloc(1, sizeof(struct handler));
  if (!handler) {
    trace_errno(trace);
    return NULL;
  }

  /*FIXME size guesses*/
  handler->elf_interpeters = create_set(1, trace);
  if (!ok(trace)) {
    free(handler);
    return NULL;
  }

  handler->handled_executables = create_set(1000, trace);
  if (!ok(trace)) {
    free(handler);
    return NULL;
  }

  handler->config_path = strdup(config_path);
  if (!handler->config_path) {
    trace_errno(trace);
    free(handler);
    return NULL;
  }

  handler->config = load_config(config_path, trace);
  if (!ok(trace)) {
    trace_static(messages.handler.config.cannot_load, trace);
    free(handler);
    return NULL;
  }

  handler->store =
      create_store(get_configured_store_root(handler->config), trace);
  if (!ok(trace)) {
    free(handler);
    return NULL;
  }

  handler->linq =
      load_linq(get_configured_queue_path(handler->config),
                get_configured_debounce_seconds(handler->config), trace);
  if (!ok(trace)) {
    trace_static(messages.handler.linq.cannot_load, trace);
    free(handler);
    return NULL;
  }

  handler->editor_pid_bitmap =
      create_bitmap(get_configured_max_pid_guess(handler->config), trace);
  if (!ok(trace)) {
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

  if (!is_in_set(file_path, handler->elf_interpeters)) {
    if (is_in_set(exe_filename, get_configured_editors(handler->config))) {
      set_bit_in_bitmap(pid, handler->editor_pid_bitmap, trace);
    } else {
      unset_bit_in_bitmap(pid, handler->editor_pid_bitmap);
    }

    if (!is_in_set(file_path, handler->handled_executables)) {
      char *interpreter = get_elf_interpreter(fd, trace);
      if (ok(trace) && interpreter) {
        add_to_set(interpreter, handler->elf_interpeters, trace);
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
      return free(file_path);
    }
  }

  if (!strcmp(file_path, handler->config_path)) {
    struct config *new_config = load_config(handler->config_path, trace);
    if (!ok(trace)) {
      trace_static(messages.handler.config.cannot_reload, trace);
      return free(file_path);
    }

    struct linq *new_linq =
        load_linq(get_configured_queue_path(new_config),
                  get_configured_debounce_seconds(new_config), trace);
    if (!ok(trace)) {
      trace_static(messages.handler.linq.cannot_reload, trace);
      free_config(new_config);
      return free(file_path);
    }

    struct store *new_store =
        create_store(get_configured_store_root(new_config), trace);
    if (!ok(trace)) {
      free_linq(new_linq);
      free_config(new_config);
      return free(file_path);
    }

    free_config(handler->config);
    handler->config = new_config;
    free_store(handler->store);
    handler->store = new_store;
    free_linq(handler->linq);
    handler->linq = new_linq;
  }
}

void handle_timeout(struct handler *handler, time_t *retry_after_seconds,
                    struct trace *trace) {
  *retry_after_seconds = 0;
  for (;;) {
    char *path = pop_from_linq(
        handler->linq, get_configured_path_length_guess(handler->config),
        retry_after_seconds, trace);
    if (!ok(trace)) {
      trace_static(messages.handler.linq.cannot_pop, trace);
      return;
    }
    if (*retry_after_seconds) {
      return;
    }

    char *version = get_timestamp(
        get_configured_version_pattern(handler->config),
        get_configured_version_max_length(handler->config), trace);
    if (!ok(trace)) {
      free(path);
      return;
    }
    if (strchr(version, '/')) {
      trace_static(messages.handler.version.has_slashes, trace);
      free(path);
      return;
    }

    copy_to_store(path, version, handler->store, trace);
    if (get_trace_message(trace) == messages.store.copy.file_does_not_exist ||
        get_trace_message(trace) == messages.store.copy.permission_denied) {
      clear(trace);
    }
    if (!ok(trace)) {
      trace_static(messages.handler.store.cannot_copy, trace);
      free(path);
      free(version);
      return;
    }
    free(path);
    free(version);
  }
}

void free_handler(struct handler *handler) {
  if (handler) {
    free(handler->config_path);
    free_config(handler->config);
    free_linq(handler->linq);
    free_store(handler->store);
    free_bitmap(handler->editor_pid_bitmap);
    free_set(handler->elf_interpeters);
    free_set(handler->handled_executables);
    free(handler);
  }
}
