#include "bitmap.h"
#include "config.h"
#include "deref.h"
#include "set.h"
#include "store.h"
#include "timestamp.h"

#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fanotify.h>
#include <unistd.h>

enum exit_code {
  CODE_SUCCESS,
  CODE_STORE,
  CODE_FANOTIFY,
  CODE_PROC,
  CODE_TIME,
  CODE_MEMORY,
  CODE_CONFIG,
};

static const size_t version_max_size = 80;

struct error {
  const char *message;
  const char *context;
  bool is_errno_contextless;
  bool is_ok;
};

static void handle(struct error *error) {
  int saved_errno = errno;
  if (!error->is_ok) {
    fprintf(stderr, "+ ");
  }
  fprintf(stderr, "%s", error->message);
  if (error->context) {
    fprintf(stderr, " (%s)", error->context);
  }
  if (!error->is_errno_contextless || !error->context) {
    /*TODO strerror is not thread-safe*/
    fprintf(stderr, ": %s", strerror(saved_errno));
  }
  fprintf(stderr, "\n");
  error->is_ok = false;
}

static void error_callback_function(void *error) { handle(error); }

int main(int argc, const char **argv) {
  struct error error = {"Cannot create callback", NULL, false, true};
  struct callback *error_callback =
      create_callback(error_callback_function, &error, NULL);
  if (!error_callback) {
    handle(&error);
    return CODE_MEMORY;
  }

  error.message = "Cannot create PID bitmap";
  struct bitmap *editor_pid_bitmap = create_bitmap(1 << 16, error_callback);
  if (!error.is_ok) {
    return CODE_MEMORY;
  }

  const char *config_path = argc > 2 ? argv[2] : "./config.lua";
  error.message = "Cannot load configuration";
  error.is_errno_contextless = true;
  struct config *config =
      load_config(config_path, error_callback, &error.context);
  if (!error.is_ok) {
    return CODE_CONFIG;
  }
  error.is_errno_contextless = false;

  const char *store_root = argc > 1 ? argv[1] : "./klunok-store";
  error.context = store_root;
  error.message = "Cannot create store";
  struct store *store = create_store(store_root, error_callback);
  if (!error.is_ok) {
    return CODE_STORE;
  }

  error.context = NULL;
  error.message = "Cannot init fanotify";
  int fanotify_fd = fanotify_init(FAN_CLASS_NOTIF, O_RDONLY);
  if (fanotify_fd < 0) {
    handle(&error);
    return CODE_FANOTIFY;
  }

  error.message = "Cannot mark home";
  if (fanotify_mark(fanotify_fd, FAN_MARK_ADD | FAN_MARK_FILESYSTEM,
                    FAN_OPEN_EXEC | FAN_CLOSE_WRITE, 0, "/home") < 0) {
    handle(&error);
    return CODE_FANOTIFY;
  }

  for (;;) {
    error.context = NULL;
    error.message = "Cannot read an event";
    struct fanotify_event_metadata event;
    if (read(fanotify_fd, &event, sizeof event) < sizeof event) {
      handle(&error);
      return CODE_FANOTIFY;
    }

    error.message = "Cannot resolve file path";
    char *file_path = deref_fd(event.fd, error_callback);
    if (!error.is_ok) {
      return CODE_PROC;
    }

    error.context = file_path;

    if (event.mask & FAN_OPEN_EXEC) {
      error.message = "Cannot resolve executable path";
      char *exe_filename = strrchr(file_path, '/');
      if (!exe_filename) {
        handle(&error);
        goto cleanup;
      }
      ++exe_filename;

      /*FIXME*/
      if (fnmatch("ld-linux*.so*", exe_filename, 0)) {
        if (is_in_set(exe_filename, get_configured_editors(config))) {
          error.message = "Cannot set bit in PID bitmap";
          set_bit_in_bitmap(event.pid, editor_pid_bitmap, error_callback);
          if (!error.is_ok) {
            return CODE_MEMORY;
          }
        } else {
          unset_bit_in_bitmap(event.pid, editor_pid_bitmap);
        }
      }
    } else if (event.mask & FAN_CLOSE_WRITE) {
      if (get_bit_in_bitmap(event.pid, editor_pid_bitmap) &&
          strstr(file_path, "/.") == NULL) {
        error.context = get_configured_version_pattern(config);
        error.message = "Cannot create date-based version";
        char *version = get_timestamp(get_configured_version_pattern(config),
                                      version_max_size, error_callback,
                                      &error.is_errno_contextless);
        if (!error.is_ok) {
          return CODE_TIME;
        }
        error.context = file_path;
        error.is_errno_contextless = false;

        if (strchr(version, '/')) {
          error.context = version;
          error.message = "Versions must not contain slashes";
          handle(&error);
          return CODE_CONFIG;
        }

        error.message = "Cannot copy file to store";
        copy_to_store(file_path, version, store, error_callback);
        free(version);
        error.is_ok = true;
      }
      if (!strcmp(file_path, config_path)) {
        error.is_errno_contextless = true;
        error.context = NULL;
        error.message = "Cannot reload configuration";
        struct config *new_config =
            load_config(config_path, error_callback, &error.context);
        if (error.is_ok) {
          free_config(config);
          config = new_config;
        }
        error.is_ok = true;
        error.is_errno_contextless = false;
      }
    }

  cleanup:
    free(file_path);
    close(event.fd);
  }
}
