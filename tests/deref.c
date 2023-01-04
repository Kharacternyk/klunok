#include "deref.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PATH "/dev/null"

int main() {
  int fd = open(PATH, O_RDONLY);
  int error_code = 0;
  const char *path = deref_fd(fd, &error_code);
  assert(!error_code);
  assert(path);
  assert(!strcmp(path, PATH));
}