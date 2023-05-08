#include "handler.h"
#include "trace.h"
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define CONFIG TEST_ROOT "/lua/handler.lua"
#define F1 CONFIG
#define F2 TEST_ROOT "/lua/empty.lua"
#define EMPTY "empty"
#define IN_STORE(PATH) "./klunok/store/" PATH "/version"

void test_handler(struct trace *trace) {
  struct handler *handler = load_handler(CONFIG, trace);
  assert(ok(trace));

  time_t retry_after_seconds = 0;
  handle_timeout(handler, &retry_after_seconds, trace);
  assert(ok(trace));
  assert(retry_after_seconds < 0);

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

  handle_timeout(handler, &retry_after_seconds, trace);
  assert(ok(trace));
  assert(retry_after_seconds < 0);

  assert(access(IN_STORE(F1) ".lua", F_OK) == 0);
  assert(access(IN_STORE(F2) ".lua", F_OK) == 0);

  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));

  handle_timeout(handler, &retry_after_seconds, trace);
  assert(ok(trace));
  assert(retry_after_seconds < 0);

  assert(access(IN_STORE(F2) "-1.lua", F_OK) == 0);

  close(fd);
  fd = open(EMPTY, O_CREAT, S_IRWXU);

  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));

  assert(unlink(EMPTY) >= 0);

  handle_timeout(handler, &retry_after_seconds, trace);
  assert(ok(trace));
  assert(retry_after_seconds < 0);
  assert(access(IN_STORE(EMPTY), F_OK) != 0);

  close(fd);
  fd = open(EMPTY, O_CREAT, S_IWOTH);

  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));

  handle_timeout(handler, &retry_after_seconds, trace);
  assert(ok(trace));
  assert(retry_after_seconds < 0);
  assert(access(IN_STORE(EMPTY), F_OK) != 0);

  close(fd);
  free_handler(handler);
}
