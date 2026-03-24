#include "handler.h"
#include "test-constants.h"
#include "trace.h"
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define CONFIG_BASE "lua/handler.lua"
#define CONFIG TEST_ROOT "/" CONFIG_BASE
#define F1 CONFIG
#define F2 TEST_ROOT "/lua/empty.lua"
#define EMPTY "empty"
#define IN_STORE(PATH) "./klunok/store/" PATH "/version"
#define WORKING_COPY(PATH) "./klunok/store/" PATH "/working-copy"

static void assert_symlink_target(const char *path, const char *target) {
  char actual[4096];
  ssize_t length = readlink(path, actual, sizeof(actual) - 1);
  assert(length >= 0);
  actual[length] = 0;
  assert(!strcmp(actual, target));
}

void test_handler(struct trace *trace) {
  struct handler *handler = load_handler(CONFIG, 1, trace);
  assert(ok(trace));

  time_t pause = handle_timeout(handler, trace);
  assert(ok(trace));
  assert(pause < 0);

  int fd = open(CONFIG, O_RDONLY);
  assert(fd >= 0);
  handle_open_exec(getpid(), fd, handler, trace);
  assert(ok(trace));

  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));

  close(fd);
  fd = open(F2, O_RDONLY);
  assert(fd >= 0);
  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));

  pause = handle_timeout(handler, trace);
  assert(ok(trace));
  assert(pause < 0);

  assert(access(IN_STORE(F1) ".lua", F_OK) == 0);
  assert(access(IN_STORE(F2) ".lua", F_OK) == 0);
  assert_symlink_target(WORKING_COPY(F1) ".lua", F1);
  assert_symlink_target(WORKING_COPY(F2) ".lua", F2);
  struct stat working_copy_stat;
  assert(lstat(WORKING_COPY(F2) ".lua", &working_copy_stat) == 0);

  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));

  pause = handle_timeout(handler, trace);
  assert(ok(trace));
  assert(pause < 0);

  assert(access(IN_STORE(F2) "-1.lua", F_OK) == 0);
  struct stat new_working_copy_stat;
  assert(lstat(WORKING_COPY(F2) ".lua", &new_working_copy_stat) == 0);
  assert(working_copy_stat.st_ino == new_working_copy_stat.st_ino);

  close(fd);
  fd = open(EMPTY, O_CREAT, S_IRWXU);

  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));

  assert(unlink(EMPTY) >= 0);

  pause = handle_timeout(handler, trace);
  assert(ok(trace));
  assert(pause < 0);
  assert(access(IN_STORE(EMPTY), F_OK) != 0);
  assert(access(WORKING_COPY(EMPTY), F_OK) != 0);

  close(fd);
  fd = open(EMPTY, O_CREAT, S_IWOTH);

  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));

  pause = handle_timeout(handler, trace);
  assert(ok(trace));
  assert(pause < 0);
  assert(access(IN_STORE(EMPTY), F_OK) != 0);
  assert(access(WORKING_COPY(EMPTY), F_OK) != 0);

  close(fd);
  free_handler(handler);

  handler = load_handler(CONFIG, sizeof TEST_ROOT, trace);
  assert(ok(trace));
  fd = open(CONFIG, O_RDONLY);
  assert(fd >= 0);
  handle_open_exec(getpid(), fd, handler, trace);
  assert(ok(trace));
  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));
  pause = handle_timeout(handler, trace);
  assert(pause < 0);
  assert(ok(trace));
  assert(access(IN_STORE(CONFIG_BASE) ".lua", F_OK) == 0);
  assert_symlink_target(WORKING_COPY(CONFIG_BASE) ".lua", CONFIG);

  close(fd);
  free_handler(handler);
}
