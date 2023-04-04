#include "elfinterp.h"
#include "trace.h"
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

void fuzz_elfinterp() {
  struct trace *trace = create_trace();
  assert(trace);
  get_elf_interpreter(0, trace);
}
