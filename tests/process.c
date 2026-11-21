#include "process.h"
#include "trace.h"
#include <assert.h>
#include <sys/fanotify.h>
#include <sys/pidfd.h>
#include <sys/wait.h>
#include <unistd.h>

void test_process(struct trace *trace) {
  require_pidfs(trace);
  assert(ok(trace));
  struct editors *editors = create_editors(0, trace);
  assert(ok(trace));

  pid_t editor_pid = fork();
  assert(editor_pid >= 0);
  if (!editor_pid) {
    _exit(0);
  }
  int editor_pidfd = pidfd_open(editor_pid, 0);
  pid_t next_pid = getpid();
  int next_pidfd = pidfd_open(next_pid, 0);
  assert(editor_pidfd >= 0 && next_pidfd >= 0);
  assert(!is_editor(editor_pid, editor_pidfd, editors));
  track_editor(editor_pid, editor_pidfd, editors, trace);
  assert(ok(trace));
  assert(is_editor(editor_pid, editor_pidfd, editors));
  assert(waitpid(editor_pid, NULL, 0) == editor_pid);
  assert(is_editor(editor_pid, FAN_NOPIDFD, editors));
  assert(!is_editor(next_pid, next_pidfd, editors));
  track_editor(next_pid, next_pidfd, editors, trace);
  assert(ok(trace));
  assert(is_editor(next_pid, next_pidfd, editors));
  assert(is_editor(editor_pid, FAN_NOPIDFD, editors));
  assert(!is_editor(editor_pid, next_pidfd, editors));
  assert(!is_editor(editor_pid, FAN_NOPIDFD, editors));

  free_editors(editors);
  close(editor_pidfd);
  close(next_pidfd);
}
