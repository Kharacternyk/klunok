#include "deref.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static char *deref(const char *pattern, int value,
                   struct callback *error_callback) {
  size_t max_length = 1024;

  for (;;) {
    char *link = malloc(max_length);
    char *target = malloc(max_length);
    snprintf(link, max_length, pattern, value);

    int length = readlink(link, target, max_length);

    if (length < 0) {
      invoke_callback(error_callback);
      free(link);
      free(target);
      return NULL;
    }
    if (length < max_length) {
      free(link);
      target[length] = 0;
      return target;
    }

    free(link);
    free(target);
    max_length *= 2;
  }
}

char *deref_pid(int pid, struct callback *error_callback) {
  return deref("/proc/%d/exe", pid, error_callback);
}

char *deref_fd(int fd, struct callback *error_callback) {
  return deref("/proc/self/fd/%d", fd, error_callback);
}
