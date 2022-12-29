#include "config.h"
#include "deref.h"
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

#define ERROR_STORE 1
#define ERROR_FANOTIFY 2
#define ERROR_PROC 3
#define ERROR_TIME 4
#define ERROR_MEMORY 5

#define VERSION_PATTERN "v%Y-%m-%d-%H-%M-%S"
#define VERSION_LENGTH 20

static void typed_error_callback_function(const char **message) {
  perror(*message);
  *message = NULL;
}

static void error_callback_function(void *message) {
  typed_error_callback_function(message);
}

int main(int argc, const char **argv) {
  size_t pid_array_size = 1 << 16;
  bool *pid_array = calloc(pid_array_size, 1);
  if (!pid_array) {
    return ERROR_MEMORY;
  }

  const char *error_message = NULL;
  struct callback *error_callback =
      create_callback(error_callback_function, &error_message);

  struct set *editors = get_editors();
  const char *store_root = argc > 1 ? argv[1] : "./klunok-store";
  error_message = "Cannot create store";
  struct store *store = create_store(store_root, error_callback);

  if (!error_message) {
    return ERROR_STORE;
  }

  /* TODO Why does it behave strange with O_RDWR? */
  int fanotify_fd = fanotify_init(FAN_CLASS_CONTENT, O_RDONLY);

  if (fanotify_fd < 0) {
    perror("Cannot init fanotify");
    return ERROR_FANOTIFY;
  }

  if (fanotify_mark(fanotify_fd, FAN_MARK_ADD | FAN_MARK_FILESYSTEM,
                    FAN_OPEN_EXEC_PERM | FAN_CLOSE_WRITE, 0, "/home") < 0) {
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

    if (event.mask & FAN_OPEN_EXEC_PERM) {
      char *exe_filename = strrchr(file_path, '/');
      if (!exe_filename) {
        perror("Cannot resolve executable path");
        return ERROR_PROC;
      }
      ++exe_filename;

      if (event.pid >= pid_array_size) {
        fprintf(stderr, "%s\n", "RESIZE");
        bool *new_pid_array = calloc(event.pid * 2, 1);
        if (!new_pid_array) {
          return ERROR_MEMORY;
        }
        memcpy(new_pid_array, pid_array, pid_array_size);
        pid_array_size = event.pid * 2;
        free(pid_array);
        pid_array = new_pid_array;
      }

      /*FIXME*/
      if (fnmatch("ld-linux*.so*", exe_filename, 0)) {
        pid_array[event.pid] = is_in_set(exe_filename, editors);
      }

      struct fanotify_response response = {
          .fd = event.fd,
          .response = FAN_ALLOW,
      };
      if (write(fanotify_fd, &response, sizeof response) < sizeof response) {
        perror("Cannot allow file access");
        return ERROR_FANOTIFY;
      }
    } else if (event.mask & FAN_CLOSE_WRITE) {
      if (event.pid < pid_array_size && pid_array[event.pid] &&
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

    free(file_path);
    close(event.fd);
  }
}
