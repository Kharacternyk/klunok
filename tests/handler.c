#include "handler.h"
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#define CONFIG TEST_ROOT "/configs/handler.lua"
#define EMPTY TEST_ROOT "/klunok/empty"
#define IN_STORE(PATH) TEST_ROOT "/klunok/store/" PATH "/version"

int main() {
  assert(chdir(TEST_ROOT) >= 0);

  struct trace *trace = create_trace();
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

  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));

  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));

  handle_timeout(handler, &retry_after_seconds, trace);
  assert(ok(trace));
  assert(retry_after_seconds < 0);

  assert(access(IN_STORE(CONFIG), F_OK) == 0);
  assert(unlink(IN_STORE(CONFIG)) >= 0);

  assert(access(IN_STORE(CONFIG) "-1", F_OK) == 0);
  assert(unlink(IN_STORE(CONFIG) "-1") >= 0);

  assert(access(IN_STORE(CONFIG) "-2", F_OK) == 0);
  assert(unlink(IN_STORE(CONFIG) "-2") >= 0);

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

  assert(unlink(EMPTY) >= 0);

  close(fd);
  free_handler(handler);
  free(trace);
}
