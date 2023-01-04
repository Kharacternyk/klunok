#include "deref.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *deref_fd(int fd, int *error_code) {
  size_t max_length = 1024;

  for (;;) {
    char *link = malloc(max_length);
    char *target = malloc(max_length);
    snprintf(link, max_length, "/proc/self/fd/%d", fd);

    int length = readlink(link, target, max_length);

    if (length < 0) {
      *error_code = errno;
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
