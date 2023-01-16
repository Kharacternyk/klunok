#include "deref.h"
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

int main() {
  struct trace *trace = create_trace();
  const char *path = "/dev/null";
  int fd = open(path, O_RDONLY);
  char *deref_path = deref_fd(fd, 0, trace);

  assert(ok(trace));
  assert(path);
  assert(!strcmp(deref_path, path));

  free(deref_path);
  free(trace);
}
