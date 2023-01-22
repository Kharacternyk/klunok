#include <assert.h>
#include <dlfcn.h>

int main(int argc, const char **argv) {
  assert(argc > 1);
  void *self = dlopen(0, RTLD_LAZY);
  assert(self);
  void (*test_function)() = dlsym(self, argv[1]);
  assert(test_function);
  test_function();
}
