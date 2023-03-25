#include "deref.h"
#include "buffer.h"
#include "trace.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

char *deref_fd(int fd, size_t length_guess, struct trace *trace) {
  struct buffer *link_buffer = create_buffer(trace);
  concat_string("/proc/self/fd/", link_buffer, trace);
  concat_size(fd, link_buffer, trace);
  if (!ok(trace)) {
    free_buffer(link_buffer);
    return NULL;
  }

  size_t max_size = length_guess + 1;

  for (;;) {
    char *target = TNULL(malloc(max_size), trace);
    int length = TNEG(
        readlink(get_string(get_view(link_buffer)), target, max_size), trace);

    if (!ok(trace)) {
      free_buffer(link_buffer);
      free(target);
      return NULL;
    }
    if (length < max_size) {
      free_buffer(link_buffer);
      target[length] = 0;
      return target;
    }

    free(target);
    max_size *= 2;
  }
}
