#include "deref.h"
#include "builder.h"
#include "trace.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

char *deref_fd(int fd, size_t length_guess, struct trace *trace) {
  struct builder *link_builder = create_builder(trace);
  concat_string("/proc/self/fd/", link_builder, trace);
  concat_size(fd, link_builder, trace);
  if (!ok(trace)) {
    free_builder(link_builder);
    return NULL;
  }

  size_t max_size = length_guess + 1;

  for (;;) {
    char *target = TNULL(malloc(max_size), trace);
    int length =
        TNEG(readlink(build_string(link_builder), target, max_size), trace);

    if (!ok(trace)) {
      free_builder(link_builder);
      free(target);
      return NULL;
    }
    if (length < max_size) {
      free_builder(link_builder);
      target[length] = 0;
      return target;
    }

    free(target);
    max_size *= 2;
  }
}
