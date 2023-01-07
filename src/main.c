#include "bitmap.h"
#include "config.h"
#include "deref.h"
#include "timestamp.h"

#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fanotify.h>
#include <sys/stat.h>
#include <unistd.h>

enum exit_code {
  CODE_SUCCESS,
  CODE_FANOTIFY,
  CODE_PROC,
  CODE_TIME,
  CODE_MEMORY,
  CODE_CONFIG,
  CODE_UID_GID,
};

static void report(int error_code, const char *error_message,
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
}

int main(int argc, const char **argv) {
  int fanotify_fd = fanotify_init(FAN_CLASS_NOTIF, O_RDONLY);
  if (fanotify_fd < 0) {
    report(errno, "Cannot init fanotify", NULL);
    return CODE_FANOTIFY;
  }

  if (argc < 2) {
    report(0, "No configuration file provided", "Usage: klunok CONFIG");
    return CODE_CONFIG;
  }

  char *config_path = realpath(argv[1], NULL);
  if (!config_path) {
    report(errno, "Cannot canonicalize path of configuration file", argv[1]);
    return CODE_CONFIG;
  }

  if (fanotify_mark(fanotify_fd, FAN_MARK_ADD | FAN_MARK_FILESYSTEM,
                    FAN_OPEN_EXEC | FAN_CLOSE_WRITE, 0, config_path) < 0) {
    report(errno, "Cannot watch mount point of configuration file",
           config_path);
    return CODE_FANOTIFY;
  }

  struct stat config_stat;
  if (stat(config_path, &config_stat) < 0) {
    report(errno, "Cannot stat configuration file", config_path);
    return CODE_CONFIG;
  }
  if (config_stat.st_uid == 0) {
    report(0, "Configuration file must not be owned by root", config_path);
    return CODE_CONFIG;
  }
  if (config_stat.st_gid == 0) {
    report(0, "Configuration file must not be owned by the root group",
           config_path);
    return CODE_CONFIG;
  }
  if (setgid(config_stat.st_gid) < 0) {
    report(errno, "Cannot drop privileged group ID", NULL);
    return CODE_UID_GID;
  }
  if (setuid(config_stat.st_uid) < 0) {
    report(errno, "Cannot drop privileged user ID", NULL);
    return CODE_UID_GID;
  }

  int error_code = 0;
  char *error_message = NULL;

  struct config *config = load_config(config_path, &error_code, &error_message);
  if (error_message || error_code) {
    report(error_code, "Cannot load configuration", error_message);
    return CODE_CONFIG;
  }

  struct bitmap *editor_pid_bitmap =
      create_bitmap(get_configured_max_pid_guess(config), &error_code);
  if (error_code) {
    report(error_code, "Cannot create PID bitmap", NULL);
    return CODE_MEMORY;
  }

  const pid_t self = getpid();

  for (;;) {
    struct fanotify_event_metadata event;
    if (read(fanotify_fd, &event, sizeof event) < sizeof event) {
      report(errno, "Cannot read a fanotify event", NULL);
      return CODE_FANOTIFY;
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
            report(error_code, "Cannot set bit in pid bitmap", NULL);
            return CODE_MEMORY;
          }
        } else {
          unset_bit_in_bitmap(event.pid, editor_pid_bitmap);
        }
      }
    } else if (event.mask & FAN_CLOSE_WRITE && event.pid != self) {
      if (get_bit_in_bitmap(event.pid, editor_pid_bitmap) &&
          strstr(file_path, "/.") == NULL) {
        bool is_overflow = false;
        char *version = get_timestamp(get_configured_version_pattern(config),
                                      get_configured_version_max_length(config),
                                      &error_code, &is_overflow);
        if (error_code) {
          report(error_code, "Cannot create date-based version", NULL);
          return CODE_TIME;
        }
        if (is_overflow) {
          report(0, "Date-based version is too long", NULL);
          return CODE_TIME;
        }
        if (strchr(version, '/')) {
          report(0, "Versions must not contain slashes", version);
          return CODE_CONFIG;
        }

        int cleanup_error_code = 0;
        copy_to_store(file_path, version, get_configured_store(config),
                      &error_code, &cleanup_error_code);
        if (error_code) {
          report(error_code, "Cannot copy file to store", file_path);
          if (cleanup_error_code) {
            report(cleanup_error_code,
                   "Cannot cleanup after unsuccessful copy to store",
                   file_path);
          }
        }
        free(version);
        error_code = 0;
      }
      if (!strcmp(file_path, config_path)) {
        struct config *new_config =
            load_config(config_path, &error_code, &error_message);
        if (!error_message && !error_code) {
          free_config(config);
          config = new_config;
        } else {
          report(error_code, "Cannot reload configuration", error_message);
          free(error_message);
          error_code = 0;
          error_message = NULL;
        }
      }
    }

  cleanup:
    free(file_path);
    close(event.fd);
  }
}
