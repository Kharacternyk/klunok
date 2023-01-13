#include "handler.h"
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

/* TODO error reporting is awful, we need to come up with a way for low-level
 * functions to return contextful errors */

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

  struct handler *handler =
      load_handler(config_path, &error_code, &error_message);
  if (error_code) {
    return report(error_code, "Cannot load configuration or application state",
                  error_message);
  }

  pid_t self = getpid();
  time_t wake_after_seconds = -1;
  struct pollfd pollfd = {
      .fd = fanotify_fd,
      .events = POLLIN,
      .revents = 0,
  };

  for (;;) {
    int status = poll(&pollfd, 1, wake_after_seconds * 1000);
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

      char *error_message = NULL;
      int error_code = 0;

      if (event.mask & FAN_OPEN_EXEC) {
        handle_open_exec(event.pid, event.fd, handler, &error_code);
        if (error_code) {
          report(error_code, "Cannot handle FAN_OPEN_EXEC", NULL);
        }
      } else if (event.mask & FAN_CLOSE_WRITE && event.pid != self) {
        bool is_config_changed = false;
        handle_close_write(event.pid, event.fd, handler, &is_config_changed,
                           &error_code, &error_message);
        if (error_code || error_message) {
          if (is_config_changed) {
            report(error_code,
                   "Cannot reload configuration or application state",
                   error_message);
          } else {
            report(error_code, "Cannot handle FAN_CLOSE_WRITE", error_message);
          }
        }
      }

      free(error_message);
      close(event.fd);
    }

    int error_code = 0;
    int cleanup_error_code = 0;
    bool is_version_invalid = false;
    handle_timeout(handler, &wake_after_seconds, &is_version_invalid,
                   &error_code, &cleanup_error_code);
    if (is_version_invalid) {
      report(error_code, "Version pattern produced an invalid version", NULL);
    }
    if (error_code) {
      report(error_code, "Cannot handle timeout", NULL);
    }
    if (cleanup_error_code) {
      report(cleanup_error_code,
             "Cannot clean up after unsuccessful copy to store", NULL);
    }
  }
}
