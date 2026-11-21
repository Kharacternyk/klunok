#include "process.h"
#include "messages.h"
#include "trace.h"
#include <fcntl.h>
#include <linux/magic.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/fanotify.h>
#include <sys/pidfd.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <unistd.h>

void require_pidfs(struct trace *trace) {
  int fd = TNEG(pidfd_open(getpid(), 0), trace);
  if (ok(trace)) {
    struct statfs fs;
    TNEG(fstatfs(fd, &fs), trace);

    if (ok(trace) && fs.f_type != PID_FS_MAGIC) {
      throw_static(messages.main.pidfs_required, trace);
    }

    close(fd);
  }
}

/* Both descriptors must be live references to pidfs inodes. */
bool same_process(int first_pidfd, int second_pidfd) {
  struct stat first, second;
  return first_pidfd >= 0 && second_pidfd >= 0 &&
         fstat(first_pidfd, &first) == 0 && fstat(second_pidfd, &second) == 0 &&
         first.st_dev == second.st_dev && first.st_ino == second.st_ino;
}

struct editors {
  /* Zero for an empty slot, pidfd + 1 for a tracked editor, or
     FAN_NOPIDFD for an exited editor whose writes may still be queued. */
  int *slots;
  size_t size;
  int exits;
};

struct editors *create_editors(size_t size_guess, struct trace *trace) {
  struct editors *editors = TNULL(malloc(sizeof *editors), trace);
  int *slots = TNULL(calloc(size_guess, sizeof *slots), trace);
  int exits = TNEG(epoll_create1(EPOLL_CLOEXEC), trace);
  if (!ok(trace)) {
    free(slots);
    free(editors);
    return NULL;
  }
  *editors =
      (struct editors){.slots = slots, .size = size_guess, .exits = exits};
  return editors;
}

void forget_editor(pid_t pid, struct editors *editors) {
  if (pid <= 0 || (size_t)pid >= editors->size) {
    return;
  }
  int fd = editors->slots[pid] - 1;
  if (fd >= 0) {
    epoll_ctl(editors->exits, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
  }
  editors->slots[pid] = 0;
}

bool is_editor(pid_t pid, int pidfd, struct editors *editors) {
  if (pid <= 0 || (size_t)pid >= editors->size || !editors->slots[pid]) {
    return false;
  }
  if (pidfd < 0) {
    return pidfd == FAN_NOPIDFD;
  }
  if (same_process(pidfd, editors->slots[pid] - 1)) {
    return true;
  }
  forget_editor(pid, editors);
  return false;
}

void track_editor(pid_t pid, int pidfd, struct editors *editors,
                  struct trace *trace) {
  if (pid <= 0 || !ok(trace)) {
    return;
  }
  if (pidfd < 0 && pidfd != FAN_NOPIDFD) {
    forget_editor(pid, editors);
    return;
  }

  /* Keep reaped editors' PIDs for queued final writes. */
  struct epoll_event exited[16];
  int count;
  while ((count = TNEG(epoll_wait(editors->exits, exited, 16, 0), trace)) > 0) {
    for (int i = 0; i < count; ++i) {
      pid_t exited_pid = exited[i].data.u32;
      forget_editor(exited_pid, editors);
      editors->slots[exited_pid] = FAN_NOPIDFD;
    }
  }
  if (!ok(trace) || (pidfd >= 0 && is_editor(pid, pidfd, editors))) {
    return;
  }

  if ((size_t)pid >= editors->size) {
    size_t size = 2 * ((size_t)pid + 1);
    int *slots = TNULL(calloc(size, sizeof *slots), trace);
    if (!ok(trace)) {
      return;
    }
    memcpy(slots, editors->slots, editors->size * sizeof *slots);
    free(editors->slots);
    editors->slots = slots;
    editors->size = size;
  }
  forget_editor(pid, editors);
  if (pidfd == FAN_NOPIDFD) {
    editors->slots[pid] = FAN_NOPIDFD;
    return;
  }
  int fd = TNEG(fcntl(pidfd, F_DUPFD_CLOEXEC, 0), trace);
  /* A zombie can still have a pidfd in a fanotify event until it is reaped. */
  struct epoll_event watch = {.events = EPOLLHUP, .data.u32 = pid};
  if (fd >= 0) {
    if (TNEG(epoll_ctl(editors->exits, EPOLL_CTL_ADD, fd, &watch), trace) < 0) {
      close(fd);
      return;
    }
    editors->slots[pid] = fd + 1;
  }
}

void free_editors(struct editors *editors) {
  if (editors) {
    for (size_t i = 0; i < editors->size; ++i) {
      if (editors->slots[i] > 0) {
        close(editors->slots[i] - 1);
      }
    }
    close(editors->exits);
    free(editors->slots);
    free(editors);
  }
}
