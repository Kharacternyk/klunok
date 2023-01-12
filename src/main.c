#include "bitmap.h"
#include "config.h"
#include "deref.h"
#include "linq.h"
#include "store.h"
#include "timestamp.h"

#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fanotify.h>
#include <sys/stat.h>
#include <unistd.h>

static int report(int error_code, const char *error_message,
                  const char *error_context) {
  fprintf(stderr, "%s", error_message);
  if (error_context) {
    fprintf(stderr, " (%s)", error_context);
  }
  if (error_code) {
    /*TODO strerror is not thread-safe*/
    fprintf(stderr, ": %s", strerror(error_code));
  }
  fprintf(stderr, "\n");
  return error_code ? error_code : -1;
}

int main(int argc, const char **argv) {
  int fanotify_fd = fanotify_init(FAN_CLASS_NOTIF, O_RDONLY);
  if (fanotify_fd < 0) {
    return report(errno, "Cannot init fanotify", NULL);
  }

  if (argc < 2) {
    return report(0, "No configuration file provided", "Usage: klunok CONFIG");
  }

  char *config_path = realpath(argv[1], NULL);
  if (!config_path) {
    return report(errno, "Cannot canonicalize path of configuration file",
                  argv[1]);
  }

  if (fanotify_mark(fanotify_fd, FAN_MARK_ADD | FAN_MARK_FILESYSTEM,
                    FAN_OPEN_EXEC | FAN_CLOSE_WRITE, 0, config_path) < 0) {
    return report(errno, "Cannot watch mount point of configuration file",
                  config_path);
  }

  struct stat config_stat;
  if (stat(config_path, &config_stat) < 0) {
    return report(errno, "Cannot stat configuration file", config_path);
  }
  if (config_stat.st_uid == 0) {
    return report(0, "Configuration file must not be owned by root",
                  config_path);
  }
  if (config_stat.st_gid == 0) {
    return report(0, "Configuration file must not be owned by the root group",
                  config_path);
  }
  if (setgid(config_stat.st_gid) < 0) {
    return report(errno, "Cannot drop privileged group ID", NULL);
  }
  if (setuid(config_stat.st_uid) < 0) {
    return report(errno, "Cannot drop privileged user ID", NULL);
  }

  int error_code = 0;
  char *error_message = NULL;

  struct config *config = load_config(config_path, &error_code, &error_message);
  if (error_message || error_code) {
    return report(error_code, "Cannot load configuration", error_message);
  }

  struct store *store =
      create_store(get_configured_store_root(config), &error_code);
  if (error_code) {
    return report(error_code, "Cannot create store",
                  get_configured_store_root(config));
  }

  struct linq *linq =
      load_linq(get_configured_queue_path(config),
                get_configured_debounce_seconds(config), &error_code);
  if (error_code) {
    return report(error_code, "Cannot load queue",
                  get_configured_queue_path(config));
  }

  struct bitmap *editor_pid_bitmap =
      create_bitmap(get_configured_max_pid_guess(config), &error_code);
  if (error_code) {
    return report(error_code, "Cannot create PID bitmap", NULL);
  }

  pid_t self = getpid();
  time_t wake_after_seconds = -1;
  struct pollfd pollfd = {
      .fd = fanotify_fd,
      .events = POLLIN,
      .revents = 0,
  };

  for (;;) {
    int status = poll(&pollfd, 1, wake_after_seconds);
    if (status < 0) {
      return report(errno, "Cannot poll fanotify file descriptor", NULL);
    }
    if (status > 0 && pollfd.revents ^ POLLIN) {
      return report(0, "Cannot poll fanotify file descriptor", NULL);
    }
    if (status > 0) {
      struct fanotify_event_metadata event;
      if (read(fanotify_fd, &event, sizeof event) < sizeof event) {
        return report(errno, "Cannot read a fanotify event", NULL);
      }
      if (event.vers != FANOTIFY_METADATA_VERSION) {
        return report(0, "Kernel fanotify version does not match headers",
                      NULL);
      }

      char *file_path = deref_fd(
          event.fd, get_configured_path_length_guess(config), &error_code);
      if (error_code) {
        report(error_code, "Cannot dereference file path", NULL);
        goto cleanup;
      }

      if (event.mask & FAN_OPEN_EXEC) {
        char *exe_filename = strrchr(file_path, '/');
        if (!exe_filename) {
          report(0, "Cannot get executable name", file_path);
          goto cleanup;
        }
        ++exe_filename;

        /*FIXME*/
        if (fnmatch("ld-linux*.so*", exe_filename, 0)) {
          if (is_in_set(exe_filename, get_configured_editors(config))) {
            set_bit_in_bitmap(event.pid, editor_pid_bitmap, &error_code);
            if (error_code) {
              return report(error_code, "Cannot set bit in pid bitmap", NULL);
            }
          } else {
            unset_bit_in_bitmap(event.pid, editor_pid_bitmap);
          }
        }
      } else if (event.mask & FAN_CLOSE_WRITE && event.pid != self) {
        if (get_bit_in_bitmap(event.pid, editor_pid_bitmap) &&
            strstr(file_path, "/.") == NULL) {
          push_to_linq(file_path, linq, &error_code);
          if (error_code) {
            return report(error_code, "Cannot push to queue", NULL);
          }
        }
        if (!strcmp(file_path, config_path)) {
          struct config *new_config =
              load_config(config_path, &error_code, &error_message);
          if (error_message || error_code) {
            report(error_code, "Cannot reload configuration", error_message);
            free(error_message);
            error_code = 0;
            error_message = NULL;
          } else {
            free_config(config);
            config = new_config;

            struct linq *new_linq =
                load_linq(get_configured_queue_path(config),
                          get_configured_debounce_seconds(config), &error_code);
            if (error_code) {
              report(error_code, "Cannot reload queue",
                     get_configured_queue_path(config));
              error_code = 0;
            } else {
              free_linq(linq);
              linq = new_linq;
            }

            struct store *new_store =
                create_store(get_configured_store_root(config), &error_code);
            if (error_code) {
              report(error_code, "Cannot recreate store",
                     get_configured_store_root(config));
              error_code = 0;
            } else {
              free_store(store);
              store = new_store;
            }
          }
        }
      }

    cleanup:
      free(file_path);
      close(event.fd);
    }

    wake_after_seconds = 0;
    for (;;) {
      /*FIXME qualifiers*/
      char *path = pop_from_linq(linq, get_configured_path_length_guess(config),
                                 &wake_after_seconds, &error_code);
      if (error_code) {
        return report(error_code, "Cannot pop from queue", NULL);
      }
      if (wake_after_seconds) {
        break;
      }
      struct stat path_stat;
      if (stat(path, &path_stat) < 0) {
        report(errno, "Cannot stat head of queue", NULL);
        free(path);
        break;
      }

      if (time(NULL) - path_stat.st_mtime <
          get_configured_debounce_seconds(config)) {
        push_to_linq(path, linq, &error_code);
        if (error_code) {
          return report(error_code, "Cannot push to queue", NULL);
        }
      }

      bool is_overflow = false;
      char *version = get_timestamp(get_configured_version_pattern(config),
                                    get_configured_version_max_length(config),
                                    &error_code, &is_overflow);
      if (error_code) {
        return report(error_code, "Cannot create date-based version", NULL);
      }
      if (is_overflow) {
        return report(0, "Date-based version is too long", NULL);
      }
      if (strchr(version, '/')) {
        return report(0, "Versions must not contain slashes", version);
      }

      int cleanup_error_code = 0;
      copy_to_store(path, version, store, &error_code, &cleanup_error_code);
      if (error_code) {
        report(error_code, "Cannot copy file to store", path);
        if (cleanup_error_code) {
          report(cleanup_error_code,
                 "Cannot cleanup after unsuccessful copy to store", path);
        }
      }
      free(path);
      free(version);
      error_code = 0;
    }
  }
}
