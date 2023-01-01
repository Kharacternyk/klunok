#include "bitmap.h"
#include "deref.h"
#include "filelines.h"
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

enum error {
  ERROR_STORE,
  ERROR_FANOTIFY,
  ERROR_PROC,
  ERROR_TIME,
  ERROR_MEMORY,
  ERROR_CONFIG,
};

#define VERSION_PATTERN "v%Y-%m-%d-%H-%M"
#define VERSION_LENGTH 17

static void typed_error_callback_function(const char **message) {
  perror(*message);
  *message = NULL;
}

static void error_callback_function(void *message) {
  typed_error_callback_function(message);
}

int main(int argc, const char **argv) {
  const char *error_message = NULL;
  struct callback *error_callback =
      create_callback(error_callback_function, &error_message, NULL);
  if (!error_callback) {
    perror("Cannot create error callback");
    return ERROR_MEMORY;
  }

  error_message = "Cannot create PID bitmap";
  struct bitmap *editor_pid_bitmap = create_bitmap(1 << 16, error_callback);
  if (!error_message) {
    return ERROR_MEMORY;
  }

  const char *editors_path = argc > 2 ? argv[2] : "./editors";
  error_message = "Cannot read editors";
  struct set *editors = get_lines(editors_path, 8, error_callback);
  if (!error_message) {
    return ERROR_CONFIG;
  }

  const char *store_root = argc > 1 ? argv[1] : "./klunok-store";
  error_message = "Cannot create store";
  struct store *store = create_store(store_root, error_callback);
  if (!error_message) {
    return ERROR_STORE;
  }

  /* TODO Why does it behave strange with O_RDWR? */
  int fanotify_fd = fanotify_init(FAN_CLASS_NOTIF, O_RDONLY);

  if (fanotify_fd < 0) {
    perror("Cannot init fanotify");
    return ERROR_FANOTIFY;
  }

  if (fanotify_mark(fanotify_fd, FAN_MARK_ADD | FAN_MARK_FILESYSTEM,
                    FAN_OPEN_EXEC | FAN_CLOSE_WRITE, 0, "/home") < 0) {
    perror("Cannot mark /home");
    return ERROR_FANOTIFY;
  }

  for (;;) {
    struct fanotify_event_metadata event;
    if (read(fanotify_fd, &event, sizeof event) < sizeof event) {
      perror("Cannot read an event");
      return ERROR_FANOTIFY;
    }

    error_message = "Cannot resolve file path";
    char *file_path = deref_fd(event.fd, error_callback);
    if (!error_message) {
      return ERROR_PROC;
    }

    if (event.mask & FAN_OPEN_EXEC) {
      char *exe_filename = strrchr(file_path, '/');
      if (!exe_filename) {
        perror("Cannot resolve executable path");
        goto cleanup;
      }
      ++exe_filename;

      /*FIXME*/
      if (fnmatch("ld-linux*.so*", exe_filename, 0)) {
        if (is_in_set(exe_filename, editors)) {
          error_message = "Cannot set bit in PID bitmap";
          set_bit_in_bitmap(event.pid, editor_pid_bitmap, error_callback);
          if (!error_message) {
            return ERROR_MEMORY;
          }
        } else {
          unset_bit_in_bitmap(event.pid, editor_pid_bitmap);
        }
      }
    } else if (event.mask & FAN_CLOSE_WRITE) {
      if (get_bit_in_bitmap(event.pid, editor_pid_bitmap) &&
          strstr(file_path, "/.") == NULL) {
        error_message = "Cannot create date-based version";
        char *version =
            get_timestamp(VERSION_PATTERN, VERSION_LENGTH, error_callback);
        if (!error_message) {
          return ERROR_TIME;
        }

        error_message = "Cannot copy file to store";
        copy_to_store(file_path, version, store, error_callback);
        free(version);
      }
    }

  cleanup:
    free(file_path);
    close(event.fd);
  }
}
