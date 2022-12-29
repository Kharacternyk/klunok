#include "../src/deref.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PATH "/dev/null"

void error_callback_function(void *parameter) { assert(parameter); }

int main() {
  struct callback *error_callback =
      create_callback(error_callback_function, NULL);

  int fd = open(PATH, O_RDONLY);
  assert(!strcmp(deref_fd(fd, error_callback), PATH));
}
