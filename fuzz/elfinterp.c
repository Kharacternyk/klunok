#define _GNU_SOURCE
#include "elfinterp.h"
#include "trace.h"
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

int LLVMFuzzerTestOneInput(const char *input, size_t size) {
  struct trace *trace = create_trace();
  assert(trace);

  int fd = memfd_create("", O_RDWR);
  size_t total_written = 0;
  while (total_written < size) {
    size_t iter_written = write(fd, input + total_written, size);
    if (iter_written < 0) {
      assert(0);
    }
    total_written += iter_written;
  }

  lseek(fd, 0, SEEK_SET);
  free(get_elf_interpreter(fd, trace));
  close(fd);

  return 0;
}
