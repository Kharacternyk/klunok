#include "elfinterp.h"
#include "trace.h"
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

void test_elfinterp(struct trace *trace) {
  int fd = open(TEST_ROOT "/meson.build", O_RDONLY);
  assert(fd >= 0);

  assert(!get_elf_interpreter(fd, trace));
  assert(ok(trace));

  close(fd);
  fd = open("/proc/self/exe", O_RDONLY);
  assert(fd >= 0);
  char *interpreter = get_elf_interpreter(fd, trace);
  assert(interpreter);
  assert(ok(trace));

  close(fd);
  free(interpreter);
}
