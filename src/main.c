#include "handler.h"
#include "mountinfo.h"
#include <grp.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fanotify.h>
#include <sys/stat.h>
#include <unistd.h>

static int unwind(struct trace *trace) {
  int depth = -1;
  while (get_trace_message(trace)) {
    if (depth >= 0) {
      fprintf(stderr, "%*s└─┤because of│ ", depth * 2, "");
    }
    fprintf(stderr, "%s\n", get_trace_message(trace));
    pop_trace_message(trace);
    ++depth;
  }
  if (get_dropped_trace_message_count(trace)) {
    if (depth >= 0) {
      fprintf(stderr, "%*s└─┤", depth * 2, "");
    } else {
      fprintf(stderr, "│");
    }
    fprintf(stderr, "%zd messages dropped│\n",
            get_dropped_trace_message_count(trace));
  }
  catch_all(trace);
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
    throw_errno(trace);
    throw_static("Cannot init fanotify", trace);
    return unwind(trace);
  }

  struct mountinfo *mountinfo = create_mountinfo(trace);
  if (!ok(trace)) {
    throw_static("Cannot list mount points of the system", trace);
    return unwind(trace);
  }

  for (const char *mount = get_next_block_mount(mountinfo); mount;
       mount = get_next_block_mount(mountinfo)) {
    if (fanotify_mark(fanotify_fd, FAN_MARK_ADD | FAN_MARK_FILESYSTEM,
                      FAN_OPEN_EXEC | FAN_CLOSE_WRITE, 0, mount) < 0) {
      throw_errno(trace);
      throw_static("Cannot watch a mount point", trace);
      return unwind(trace);
    }
  }

  char *config_path = NULL;

  if (argc > 1) {
    config_path = realpath(argv[1], NULL);
    if (!config_path) {
      throw_errno(trace);
      throw_static("Cannot canonicalize path of configuration file", trace);
      return unwind(trace);
    }
  }

  const char *unprivileged_paths[] = {
      config_path,
      ".",
      "klunok",
  };
  for (size_t i = 0; i < sizeof unprivileged_paths / sizeof(char *); ++i) {
    struct stat drop_stat;
    if (unprivileged_paths[i] && stat(unprivileged_paths[i], &drop_stat) >= 0 &&
        drop_stat.st_gid && drop_stat.st_uid) {
      TNEG(setgroups(0, NULL), trace);
      TNEG(setgid(drop_stat.st_gid), trace);
      TNEG(setuid(drop_stat.st_uid), trace);
      break;
    }
  }
  if (!ok(trace) || !getuid() || !getgid()) {
    throw_static("Cannot drop privileges", trace);
    return unwind(trace);
  }

  struct handler *handler = load_handler(config_path, trace);
  if (!ok(trace)) {
    throw_static("Cannot load handler", trace);
    return unwind(trace);
  }

  pid_t self = getpid();
  time_t wake_after_seconds = 0;
  struct pollfd pollfd = {
      .fd = fanotify_fd,
      .events = POLLIN,
      .revents = 0,
  };

  for (;;) {
    int status = poll(&pollfd, 1, wake_after_seconds * 1000);
    if (status < 0) {
      throw_errno(trace);
      throw_static("Cannot poll fanotify file descriptor", trace);
      return unwind(trace);
    }
    if (status > 0 && pollfd.revents ^ POLLIN) {
      throw_errno(trace);
      throw_static("Cannot poll fanotify file descriptor", trace);
      return unwind(trace);
    }
    if (status > 0) {
      struct fanotify_event_metadata event;
      if (read(fanotify_fd, &event, sizeof event) < sizeof event) {
        throw_errno(trace);
        throw_static("Cannot read a fanotify event", trace);
        return unwind(trace);
      }
      if (event.vers != FANOTIFY_METADATA_VERSION) {
        throw_errno(trace);
        throw_static("Kernel fanotify version does not match headers", trace);
        return unwind(trace);
      }

      if (event.mask & FAN_OPEN_EXEC) {
        handle_open_exec(event.pid, event.fd, handler, trace);
        if (!ok(trace)) {
          throw_static("Cannot handle FAN_OPEN_EXEC", trace);
          return unwind(trace);
        }
      } else if (event.mask & FAN_CLOSE_WRITE && event.pid != self) {
        handle_close_write(event.pid, event.fd, handler, trace);
        if (!ok(trace)) {
          throw_static("Cannot handle FAN_CLOSE_WRITE", trace);
          return unwind(trace);
        }
      }

      close(event.fd);
    }

    handle_timeout(handler, &wake_after_seconds, trace);
    if (!ok(trace)) {
      throw_static("Cannot handle periodical tasks", trace);
      return unwind(trace);
    }
  }
}
