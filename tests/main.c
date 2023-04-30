#include "buffer.h"
#include "logstep.h"
#include "trace.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, const char **argv) {
  assert(argc > 1);
  void *self = dlopen(0, RTLD_LAZY);
  assert(self);
  void (*test_function)() = dlsym(self, argv[1]);
  assert(test_function);

  const char *xdg_run_dir = getenv("XDG_RUNTIME_DIR");
  if (!xdg_run_dir) {
    xdg_run_dir = ".";
  }

  struct trace *trace = create_trace();
  assert(trace);
  struct buffer *cwd_buffer = create_buffer(trace);
  concat_string(xdg_run_dir, cwd_buffer, trace);
  concat_string("/klunok-test-XXXXXX", cwd_buffer, trace);
  assert(ok(trace));
  free(trace);

  char *cwd = free_outer_buffer(cwd_buffer);
  assert(mkdtemp(cwd));
  logstep(2, "CWD", cwd, 0);
  assert(chdir(cwd) >= 0);
  free(cwd);

  test_function();
}
