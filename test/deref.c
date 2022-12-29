#include "../src/deref.h"
#include "unistd.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define PATH "/dev/null"

void error_callback_function(void *parameter) { assert(parameter); }

int main() {
  struct callback *error_callback =
      create_callback(error_callback_function, NULL);

  int fd = open(PATH, O_RDONLY);
  assert(!strcmp(deref_fd(fd, error_callback), PATH));

  int pid = getpid();
  assert(deref_pid(pid, error_callback));
}
