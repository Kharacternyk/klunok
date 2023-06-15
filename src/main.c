#include "buffer.h"
#include "handler.h"
#include "list.h"
#include "logstep.h"
#include "messages.h"
#include "mountinfo.h"
#include "params.h"
#include "parents.h"
#include "set.h"
#include "trace.h"
#include <assert.h>
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
  assert(peek(get_write_mounts(params)));
  assert(peek(get_exec_mounts(params)));
  assert(get_privilege_dropping_path(params));

  int fanotify_fd = fanotify_init(FAN_CLASS_NOTIF, O_RDONLY);
  if (fanotify_fd < 0) {
    throw_errno(trace);
    throw_static(messages.main.fanotify.cannot_init, trace);
    return fail(trace);
  }

  struct mountinfo *mountinfo = load_mountinfo(trace);
  if (!ok(trace)) {
    throw_static(messages.main.mount.cannot_list, trace);
    return fail(trace);
  }

  char *previous_mount = NULL;
  size_t common_parent_path_length = 0;

  for (const struct list_item *write_mount = peek(get_write_mounts(params));
       write_mount; write_mount = get_next(write_mount)) {
    char *mount = make_mount(get_value(write_mount), mountinfo, trace);
    if (TNEG(fanotify_mark(fanotify_fd, FAN_MARK_ADD | FAN_MARK_MOUNT,
                           FAN_CLOSE_WRITE, 0, mount),
             trace) < 0) {
      throw_context(get_value(write_mount), trace);
      throw_static(messages.main.mount.cannot_watch, trace);
      return fail(trace);
    }

    if (previous_mount) {
      size_t length = get_common_parent_path_length(previous_mount, mount);
      if (length < common_parent_path_length) {
        common_parent_path_length = length;
      }
      free(previous_mount);
    } else {
      previous_mount = mount;
      common_parent_path_length = strlen(mount) + 1;
    }

    if (!get_next(write_mount)) {
      free(mount);
      previous_mount = NULL;
    }
  }

  for (const struct list_item *exec_mount = peek(get_exec_mounts(params));
       exec_mount; exec_mount = get_next(exec_mount)) {
    char *mount = make_mount(get_value(exec_mount), mountinfo, trace);
    if (TNEG(fanotify_mark(fanotify_fd, FAN_MARK_ADD | FAN_MARK_MOUNT,
                           FAN_OPEN_EXEC, 0, mount),
             trace) < 0) {
      throw_context(get_value(exec_mount), trace);
      throw_static(messages.main.mount.cannot_watch, trace);
      return fail(trace);
    }
    free(mount);
  }

  struct stat drop_stat;
  if (stat(get_privilege_dropping_path(params), &drop_stat) >= 0 &&
      drop_stat.st_gid && drop_stat.st_uid) {
    TNEG(setgroups(0, NULL), trace);
    TNEG(setgid(drop_stat.st_gid), trace);
    TNEG(setuid(drop_stat.st_uid), trace);
  }

  if (!ok(trace) || !getuid() || !getgid()) {
    throw_context(get_privilege_dropping_path(params), trace);
    throw_static(messages.main.cannot_drop_privileges, trace);
    return fail(trace);
  }

  struct handler *handler =
      load_handler(get_config_path(params), common_parent_path_length, trace);
  if (!ok(trace)) {
    throw_static(messages.main.cannot_load_handler, trace);
    return fail(trace);
  }

  pid_t self = getpid();
  time_t pause = 0;
  struct pollfd pollfd = {
      .fd = fanotify_fd,
      .events = POLLIN,
      .revents = 0,
  };

  for (;;) {
    int status = poll(&pollfd, 1, pause * 1000);
    if (status < 0 || (status > 0 && pollfd.revents ^ POLLIN)) {
      throw_errno(trace);
      throw_static(messages.main.fanotify.cannot_poll, trace);
    } else if (status > 0) {
      struct fanotify_event_metadata event;
      try(trace);
      TNEG(read(fanotify_fd, &event, sizeof event) - sizeof event, trace);
      finally_rethrow_static(messages.main.fanotify.cannot_read_event, trace);
      if (ok(trace)) {
        if (event.vers != FANOTIFY_METADATA_VERSION) {
          throw_static(messages.main.fanotify.version_mismatch, trace);
        } else if (event.mask & FAN_Q_OVERFLOW) {
          throw_static(messages.main.fanotify.queue_overflow, trace);
        } else {
          if (event.mask & FAN_OPEN_EXEC) {
            try(trace);
            handle_open_exec(event.pid, event.fd, handler, trace);
            finally_rethrow_static(messages.main.cannot_handle_exec, trace);
          } else if (event.mask & FAN_CLOSE_WRITE && event.pid != self) {
            try(trace);
            handle_close_write(event.pid, event.fd, handler, trace);
            finally_rethrow_static(messages.main.cannot_handle_write, trace);
          }
          close(event.fd);
        }
      }
    }

    try(trace);
    pause = handle_timeout(handler, trace);
    finally_rethrow_static(messages.main.cannot_handle_timeout, trace);

    if (!ok(trace)) {
      return fail(trace);
    }
  }
}
