#include "buffer.h"
#include "handler.h"
#include "logstep.h"
#include "messages.h"
#include "mountinfo.h"
#include "params.h"
#include "set.h"
#include "trace.h"
#include <fcntl.h>
#include <grp.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fanotify.h>
#include <sys/stat.h>
#include <unistd.h>

static int fail(const struct trace *trace) {
  unwind(2, trace);
  return EXIT_FAILURE;
}

int main(int argc, const char **argv) {
  struct trace *trace = create_trace();
  if (!trace) {
    logstep(2, NULL, messages.main.cannot_bootstrap, 0);
    logstep(2, messages.trace.because_of, messages.main.out_of_memory, 1);
    return EXIT_FAILURE;
  }

  struct params *params = parse_params(argc, argv, trace);
  if (!ok(trace)) {
    throw_static(messages.main.cannot_parse_cli, trace);
    return fail(trace);
  }

  int fanotify_fd = fanotify_init(FAN_CLASS_NOTIF, O_RDONLY);
  if (fanotify_fd < 0) {
    throw_errno(trace);
    throw_static(messages.main.fanotify.cannot_init, trace);
    return fail(trace);
  }

  struct mountinfo *mountinfo = create_mountinfo(trace);
  if (!ok(trace)) {
    throw_static(messages.main.mount.cannot_list, trace);
    return fail(trace);
  }

  for (const char *mount = get_next_block_mount(mountinfo); mount;
       mount = get_next_block_mount(mountinfo)) {
    int fanotify_flags = 0;
    struct buffer_view *mount_view = create_buffer_view(mount, trace);
    if (!ok(trace)) {
      return fail(trace);
    }

    if (!is_within(mount_view, get_ignored_exec_mounts(params))) {
      fanotify_flags |= FAN_OPEN_EXEC;
    }
    if (!is_within(mount_view, get_ignored_write_mounts(params))) {
      fanotify_flags |= FAN_CLOSE_WRITE;
    }
    if (fanotify_flags &&
        fanotify_mark(fanotify_fd, FAN_MARK_ADD | FAN_MARK_MOUNT,
                      fanotify_flags, 0, mount) < 0) {
      throw_errno(trace);
      throw_context(mount, trace);
      throw_static(messages.main.mount.cannot_watch, trace);
      return fail(trace);
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
    throw_static(messages.main.cannot_drop_privileges, trace);
    return fail(trace);
  }

  struct handler *handler = load_handler(get_config_path(params), trace);
  if (!ok(trace)) {
    throw_static(messages.main.cannot_load_handler, trace);
    return fail(trace);
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
      throw_static(messages.main.fanotify.cannot_poll, trace);
    } else if (status > 0) {
      struct fanotify_event_metadata event;
      rethrow_check(trace);
      TNEG(read(fanotify_fd, &event, sizeof event) - sizeof event, trace);
      rethrow_static(messages.main.fanotify.cannot_read_event, trace);
      if (ok(trace)) {
        if (event.vers != FANOTIFY_METADATA_VERSION) {
          throw_static(messages.main.fanotify.version_mismatch, trace);
        } else if (event.mask & FAN_Q_OVERFLOW) {
          throw_static(messages.main.fanotify.queue_overflow, trace);
        } else {
          if (event.mask & FAN_OPEN_EXEC) {
            rethrow_check(trace);
            handle_open_exec(event.pid, event.fd, handler, trace);
            rethrow_static(messages.main.cannot_handle_exec, trace);
          } else if (event.mask & FAN_CLOSE_WRITE && event.pid != self) {
            rethrow_check(trace);
            handle_close_write(event.pid, event.fd, handler, trace);
            rethrow_static(messages.main.cannot_handle_write, trace);
          }
          close(event.fd);
        }
      }
    }

    rethrow_check(trace);
    handle_timeout(handler, &wake_after_seconds, trace);
    rethrow_static(messages.main.cannot_handle_timeout, trace);

    if (!ok(trace)) {
      return fail(trace);
    }
  }
}
