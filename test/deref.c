#include "../src/deref.h"
#include "unistd.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define PATH "/dev/null"

int main() {
  int fd = open(PATH, O_RDONLY);
  fprintf(stderr, "%s\n", deref_fd(fd));
  assert(!strcmp(deref_fd(fd), PATH));

  int pid = getpid();
  assert(deref_pid(pid));
}
