#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static const char *deref(const char *pattern, int value) {
  char *path = malloc(PATH_MAX);
  snprintf(path, PATH_MAX, pattern, value);

  int length = readlink(path, path, PATH_MAX);

  if (length < 0) {
    free(path);
    return NULL;
  }

  path[length] = 0;

  return path;
}

const char *deref_pid(int pid) { return deref("/proc/%d/exe", pid); }

const char *deref_fd(int fd) { return deref("/proc/self/fd/%d", fd); }
