#include "deref.h"
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define PATH "/dev/null"

int main() {
  int fd = open(PATH, O_RDONLY);
  int error_code = 0;
  char *path = deref_fd(fd, 0, &error_code);

  assert(!error_code);
  assert(path);
  assert(!strcmp(path, PATH));

  free(path);
}
