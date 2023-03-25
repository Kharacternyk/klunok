#include "buffer.h"
#include "handler.h"
#include "mountinfo.h"
#include "params.h"
#include "set.h"
#include "trace.h"
#include <fcntl.h>
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
      const char *prefix =
          is_trace_message_context(trace) ? "which is" : "because of";
      fprintf(stderr, "%*s└─┤%s│ ", depth * 2, "", prefix);
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

  struct params *params = parse_params(argc, argv, trace);
  if (!ok(trace)) {
    throw_static("Cannot parse command line arguments", trace);
    return unwind(trace);
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
    int fanotify_flags = 0;
    struct buffer_view *mount_view = create_buffer_view(mount, trace);
    if (!ok(trace)) {
      return unwind(trace);
    }

    if (!is_in_set(mount_view, get_ignored_exec_mounts(params))) {
      fanotify_flags |= FAN_OPEN_EXEC;
    }
    if (!is_in_set(mount_view, get_ignored_write_mounts(params))) {
      fanotify_flags |= FAN_CLOSE_WRITE;
    }
    if (fanotify_flags &&
        fanotify_mark(fanotify_fd, FAN_MARK_ADD | FAN_MARK_MOUNT,
                      fanotify_flags, 0, mount) < 0) {
      throw_errno(trace);
      throw_context(mount, trace);
      throw_static("Cannot watch a mount point", trace);
      return unwind(trace);
    }

    free_buffer_view(mount_view);
  }

  const char *privilege_dropping_path = get_privilege_dropping_path(params);
  if (!privilege_dropping_path) {
    privilege_dropping_path = ".";
  }

  struct stat drop_stat;
  if (stat(privilege_dropping_path, &drop_stat) >= 0 && drop_stat.st_gid &&
      drop_stat.st_uid) {
    TNEG(setgroups(0, NULL), trace);
    TNEG(setgid(drop_stat.st_gid), trace);
    TNEG(setuid(drop_stat.st_uid), trace);
  }

  if (!ok(trace) || !getuid() || !getgid()) {
    throw_context(privilege_dropping_path, trace);
    throw_static("Cannot drop privileges to a file owner", trace);
    return unwind(trace);
  }

  struct handler *handler = load_handler(get_config_path(params), trace);
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
    if (status < 0 || (status > 0 && pollfd.revents ^ POLLIN)) {
      throw_errno(trace);
      throw_static("Cannot poll fanotify file descriptor", trace);
    } else if (status > 0) {
      struct fanotify_event_metadata event;
      rethrow_check(trace);
      TNEG(read(fanotify_fd, &event, sizeof event) - sizeof event, trace);
      rethrow_static("Cannot read a fanotify event", trace);
      if (ok(trace)) {
        if (event.vers != FANOTIFY_METADATA_VERSION) {
          throw_static("Kernel fanotify version does not match headers", trace);
        } else if (event.mask & FAN_Q_OVERFLOW) {
          throw_static("Fanotify queue has overflowed", trace);
        } else {
          if (event.mask & FAN_OPEN_EXEC) {
            rethrow_check(trace);
            handle_open_exec(event.pid, event.fd, handler, trace);
            rethrow_static("Cannot handle FAN_OPEN_EXEC", trace);
          } else if (event.mask & FAN_CLOSE_WRITE && event.pid != self) {
            rethrow_check(trace);
            handle_close_write(event.pid, event.fd, handler, trace);
            rethrow_static("Cannot handle FAN_CLOSE_WRITE", trace);
          }
          close(event.fd);
        }
      }
    }

    rethrow_check(trace);
    handle_timeout(handler, &wake_after_seconds, trace);
    rethrow_static("Cannot handle periodical tasks", trace);

    if (!ok(trace)) {
      return unwind(trace);
    }
  }
}
