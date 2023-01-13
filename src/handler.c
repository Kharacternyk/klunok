#include "handler.h"
#include "bitmap.h"
#include "deref.h"
#include "linq.h"
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
};

struct handler *load_handler(const char *config_path, int *error_code,
                             char **error_message) {
  struct handler *handler = malloc(sizeof(struct handler));
  if (!handler) {
    *error_code = errno;
    return NULL;
  }

  handler->config_path = strdup(config_path);
  if (!handler->config_path) {
    *error_code = errno;
    goto handler_cleanup;
  }

  handler->config = load_config(config_path, error_code, error_message);
  if (*error_code || *error_message) {
    goto path_cleanup;
  }

  handler->store =
      create_store(get_configured_store_root(handler->config), error_code);
  if (*error_code) {
    goto config_cleanup;
  }

  handler->linq =
      load_linq(get_configured_queue_path(handler->config),
                get_configured_debounce_seconds(handler->config), error_code);
  if (*error_code) {
    goto store_cleanup;
  }

  handler->editor_pid_bitmap =
      create_bitmap(get_configured_max_pid_guess(handler->config), error_code);
  if (*error_code) {
    goto linq_cleanup;
  }

  return handler;

linq_cleanup:
  free_linq(handler->linq);
store_cleanup:
  free_store(handler->store);
config_cleanup:
  free_config(handler->config);
path_cleanup:
  free(handler->config_path);
handler_cleanup:
  free(handler);
  return NULL;
}

void handle_open_exec(pid_t pid, int fd, struct handler *handler,
                      int *error_code) {
  char *file_path = deref_fd(
      fd, get_configured_path_length_guess(handler->config), error_code);
  if (*error_code) {
    return;
  }
  char *exe_filename = strrchr(file_path, '/');
  assert(exe_filename);
  ++exe_filename;
  /*FIXME*/
  if (fnmatch("ld-linux*.so*", exe_filename, 0)) {
    if (is_in_set(exe_filename, get_configured_editors(handler->config))) {
      set_bit_in_bitmap(pid, handler->editor_pid_bitmap, error_code);
    } else {
      unset_bit_in_bitmap(pid, handler->editor_pid_bitmap);
    }
  }
  free(file_path);
}

void handle_close_write(pid_t pid, int fd, struct handler *handler,
                        bool *is_config_changed, int *error_code,
                        char **error_message) {
  char *file_path = deref_fd(
      fd, get_configured_path_length_guess(handler->config), error_code);
  if (*error_code) {
    return;
  }
  if (get_bit_in_bitmap(pid, handler->editor_pid_bitmap) &&
      /*FIXME*/ strstr(file_path, "/.") == NULL) {
    push_to_linq(file_path, handler->linq, error_code);
    if (*error_code) {
      goto path_cleanup;
    }
  }

  if (!strcmp(file_path, handler->config_path)) {
    *is_config_changed = true;

    struct config *new_config =
        load_config(handler->config_path, error_code, error_message);
    if (*error_message || *error_code) {
      goto path_cleanup;
    }

    struct linq *new_linq =
        load_linq(get_configured_queue_path(new_config),
                  get_configured_debounce_seconds(new_config), error_code);
    if (*error_code) {
      free_config(new_config);
      goto path_cleanup;
    }

    struct store *new_store =
        create_store(get_configured_store_root(new_config), error_code);
    if (*error_code) {
      free_linq(new_linq);
      free_config(new_config);
      goto path_cleanup;
    }

    free_config(handler->config);
    handler->config = new_config;
    free_store(handler->store);
    handler->store = new_store;
    free_linq(handler->linq);
    handler->linq = new_linq;
  }

path_cleanup:
  free(file_path);
}

void handle_timeout(struct handler *handler, time_t *retry_after_seconds,
                    bool *is_version_invalid, int *error_code,
                    int *cleanup_error_code) {
  *retry_after_seconds = 0;
  for (;;) {
    char *path = pop_from_linq(
        handler->linq, get_configured_path_length_guess(handler->config),
        retry_after_seconds, error_code);
    if (*error_code || *retry_after_seconds) {
      return;
    }

    char *version =
        get_timestamp(get_configured_version_pattern(handler->config),
                      get_configured_version_max_length(handler->config),
                      error_code, is_version_invalid);
    if (*error_code || *is_version_invalid) {
      free(path);
      return;
    }
    if (strchr(version, '/')) {
      *is_version_invalid = true;
      free(path);
      return;
    }

    bool is_not_found = false;
    copy_to_store(path, version, handler->store, error_code, cleanup_error_code,
                  &is_not_found);
    if (*error_code || *cleanup_error_code) {
      free(path);
      free(version);
      return;
    }
    free(path);
    free(version);
  }
}
