#include "handler.h"
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fanotify.h>
#include <sys/stat.h>
#include <unistd.h>

static int unwind(struct trace *trace) {
  int depth = 0;
  while (get_trace_message(trace)) {
    if (depth > 0) {
      fprintf(stderr, "%*s╰─┤because of│ ", (depth - 1) * 2, "");
    }
    fprintf(stderr, "%s\n", get_trace_message(trace));
    pop_trace_message(trace);
    ++depth;
  }
  if (get_dropped_trace_message_count(trace)) {
    if (depth > 0) {
      fprintf(stderr, "%*s╰─┤%zd messages dropped│\n", (depth - 1) * 2, "",
              get_dropped_trace_message_count(trace));
    } else {
      fprintf(stderr, "│%zd messages dropped│\n",
              get_dropped_trace_message_count(trace));
    }
  }
  clear(trace);
  return EXIT_FAILURE;
}

int main(int argc, const char **argv) {
  struct trace *trace = create_trace();
  if (!trace) {
    fprintf(stderr, "Cannot bootstrap: out of memory");
    return EXIT_FAILURE;
  }

  int fanotify_fd = fanotify_init(FAN_CLASS_NOTIF, O_RDONLY);
  if (fanotify_fd < 0) {
    trace_errno(trace);
    trace_static("Cannot init fanotify", trace);
    return unwind(trace);
  }

  if (argc < 2) {
    trace_static("No configuration file provided", trace);
    return unwind(trace);
  }

  char *config_path = realpath(argv[1], NULL);
  if (!config_path) {
    trace_errno(trace);
    trace_static("Cannot canonicalize path of configuration file", trace);
    return unwind(trace);
  }

  if (fanotify_mark(fanotify_fd, FAN_MARK_ADD | FAN_MARK_FILESYSTEM,
                    FAN_OPEN_EXEC | FAN_CLOSE_WRITE, 0, config_path) < 0) {
    trace_errno(trace);
    trace_static("Cannot watch mount point of configuration file", trace);
    return unwind(trace);
  }

  struct stat config_stat;
  if (stat(config_path, &config_stat) < 0) {
    trace_errno(trace);
    trace_static("Cannot stat configuration file", trace);
    return unwind(trace);
  }
  if (config_stat.st_uid == 0) {
    trace_errno(trace);
    trace_static("Configuration file must not be owned by root", trace);
    return unwind(trace);
  }
  if (config_stat.st_gid == 0) {
    trace_errno(trace);
    trace_static("Configuration file must not be owned by the root group",
                 trace);
    return unwind(trace);
  }
  if (setgid(config_stat.st_gid) < 0) {
    trace_errno(trace);
    trace_static("Cannot drop privileged group ID", trace);
    return unwind(trace);
  }
  if (setuid(config_stat.st_uid) < 0) {
    trace_errno(trace);
    trace_static("Cannot drop privileged user ID", trace);
    return unwind(trace);
  }

  struct handler *handler = load_handler(config_path, trace);
  if (!ok(trace)) {
    trace_static("Cannot load handler", trace);
    return unwind(trace);
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
      trace_errno(trace);
      trace_static("Cannot poll fanotify file descriptor", trace);
      return unwind(trace);
    }
    if (status > 0 && pollfd.revents ^ POLLIN) {
      trace_errno(trace);
      trace_static("Cannot poll fanotify file descriptor", trace);
      return unwind(trace);
    }
    if (status > 0) {
      struct fanotify_event_metadata event;
      if (read(fanotify_fd, &event, sizeof event) < sizeof event) {
        trace_errno(trace);
        trace_static("Cannot read a fanotify event", trace);
        return unwind(trace);
      }
      if (event.vers != FANOTIFY_METADATA_VERSION) {
        trace_errno(trace);
        trace_static("Kernel fanotify version does not match headers", trace);
        return unwind(trace);
      }

      if (event.mask & FAN_OPEN_EXEC) {
        handle_open_exec(event.pid, event.fd, handler, trace);
        if (!ok(trace)) {
          trace_static("Cannot handle FAN_OPEN_EXEC", trace);
          return unwind(trace);
        }
      } else if (event.mask & FAN_CLOSE_WRITE && event.pid != self) {
        handle_close_write(event.pid, event.fd, handler, trace);
        if (!ok(trace)) {
          trace_static("Cannot handle FAN_CLOSE_WRITE", trace);
          return unwind(trace);
        }
      }

      close(event.fd);
    }

    handle_timeout(handler, &wake_after_seconds, trace);
    if (!ok(trace)) {
      trace_static("Cannot handle periodical tasks", trace);
      return unwind(trace);
    }
  }
}
