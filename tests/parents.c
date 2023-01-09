#include "parents.h"
#include <assert.h>
#include <unistd.h>

#define DIRECTORY TEST_ROOT "/parents/abc/defgh/123"

int main() {
  int error_code = 0;

  rmdir(DIRECTORY);
  assert(access(DIRECTORY, F_OK) != 0);
  create_parents(DIRECTORY "/file", S_IRWXU, &error_code);
  assert(!error_code);
  assert(access(DIRECTORY, F_OK) == 0);
  remove_empty_parents(DIRECTORY "/file", &error_code);
  assert(!error_code);
  assert(access(DIRECTORY, F_OK) != 0);
}
