#include "handler.h"
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#define CONFIG TEST_ROOT "/configs/handler.lua"
#define CONFIG_IN_STORE TEST_ROOT "/klunok/store/" CONFIG "/version"

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
  close(fd);

  handle_timeout(handler, &retry_after_seconds, trace);
  assert(ok(trace));
  assert(retry_after_seconds < 0);

  assert(access(CONFIG_IN_STORE, F_OK) == 0);
  assert(unlink(CONFIG_IN_STORE) >= 0);

  free_handler(handler);
  free(trace);
}
