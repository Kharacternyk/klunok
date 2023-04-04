#include "elfinterp.h"
#include "trace.h"
#include <assert.h>

void fuzz_elfinterp() {
  struct trace *trace = create_trace();
  assert(trace);
  get_elf_interpreter(0, trace);
}
