#include "deref.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static char *deref(const char *pattern, int value,
                   struct callback *error_callback) {
  size_t max_length = 1024;

  for (;;) {
    char *path = malloc(max_length);
    snprintf(path, max_length, pattern, value);

    int length = readlink(path, path, max_length);

    if (length < 0) {
      invoke_callback(error_callback);
      free(path);
      return NULL;
    }
    if (length < max_length) {
      path[length] = 0;
      return path;
    }

    free(path);
    max_length *= 2;
  }
}

char *deref_pid(int pid, struct callback *error_callback) {
  return deref("/proc/%d/exe", pid, error_callback);
}

char *deref_fd(int fd, struct callback *error_callback) {
  return deref("/proc/self/fd/%d", fd, error_callback);
}
