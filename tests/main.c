#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define RUN_DIR_PATTERN "%s/klunok-test-XXXXXX"

int main(int argc, const char **argv) {
  assert(argc > 1);
  void *self = dlopen(0, RTLD_LAZY);
  assert(self);
  void (*test_function)() = dlsym(self, argv[1]);
  assert(test_function);

  const char *xdg_run_dir = getenv("XDG_RUNTIME_DIR");
  assert(xdg_run_dir);
  char *run_dir = malloc(snprintf(NULL, 0, RUN_DIR_PATTERN, xdg_run_dir) + 1);
  assert(run_dir);
  sprintf(run_dir, RUN_DIR_PATTERN, xdg_run_dir);
  assert(mkdtemp(run_dir));
  fprintf(stderr, "%s\n", run_dir);
  assert(chdir(run_dir) >= 0);
  free(run_dir);

  test_function();
}
