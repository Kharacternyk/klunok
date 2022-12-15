#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static char *deref(const char *pattern, int value) {
  size_t max_length = 1024;

  for (;;) {
    char *path = malloc(max_length);
    snprintf(path, max_length, pattern, value);

    int length = readlink(path, path, max_length);

    if (length < 0) {
      int _errno = errno;
      free(path);
      errno = _errno;
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

char *deref_pid(int pid) { return deref("/proc/%d/exe", pid); }

char *deref_fd(int fd) { return deref("/proc/self/fd/%d", fd); }
