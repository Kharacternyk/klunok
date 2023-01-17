#include "handler.h"
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  struct trace *trace = create_trace();
  struct handler *handler = load_handler(TEST_ROOT "/configs/empty.lua", trace);
  assert(ok(trace));

  time_t retry_after_seconds = 0;
  handle_timeout(handler, &retry_after_seconds, trace);
  assert(ok(trace));
  assert(retry_after_seconds < 0);

  int fd = open(TEST_ROOT "/configs/empty.lua", O_RDONLY);
  assert(fd >= 0);
  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));

  free_handler(handler);
  free(trace);
}
