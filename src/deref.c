#include "deref.h"
#include "builder.h"
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
    char *target = malloc(max_size);
    if (!target) {
      throw_errno(trace);
      free_builder(link_builder);
      return NULL;
    }

    int length = readlink(build_string(link_builder), target, max_size);

    if (length < 0) {
      throw_errno(trace);
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
