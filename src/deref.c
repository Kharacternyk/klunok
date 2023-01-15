#include "deref.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *deref_fd(int fd, size_t length_guess, struct trace *trace) {
  size_t max_size = length_guess + 1;
  char *link = NULL;
  char *target = NULL;

  for (;;) {
    char *link = malloc(max_size);
    if (!link) {
      goto fail;
    }

    char *target = malloc(max_size);
    if (!target) {
      goto fail;
    }

    if (snprintf(link, max_size, "/proc/self/fd/%d", fd) <= max_size) {
      int length = readlink(link, target, max_size);

      if (length < 0) {
        goto fail;
      }
      if (length < max_size) {
        free(link);
        target[length] = 0;
        return target;
      }
    }

    free(link);
    free(target);
    max_size *= 2;
  }

fail:
  trace_errno(trace);
  free(link);
  free(target);
  return NULL;
}
