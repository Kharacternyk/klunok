#include "linq.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define LINQ_PATH TEST_ROOT "/linq"
#define F1 TEST_ROOT "/meson.build"
#define F2 TEST_ROOT "/linq.c"

int main() {
  int error_code = 0;
  bool is_corrupted = false;
  struct linq *linq = load_linq(LINQ_PATH, &error_code, &is_corrupted);
  assert(!error_code);
  assert(!is_corrupted);

  push_to_linq(F1, linq, &error_code);
  assert(!error_code);
  bool is_empty = false;
  char *path = pop_from_linq(linq, 0, &is_empty, &error_code);
  assert(!is_empty);
  assert(!error_code);
  assert(!strcmp(path, F1));

  free(path);

  push_to_linq(F1, linq, &error_code);
  assert(!error_code);
  push_to_linq(F2, linq, &error_code);
  assert(!error_code);

  path = pop_from_linq(linq, 0, &is_empty, &error_code);
  assert(!is_empty);
  assert(!error_code);
  assert(!strcmp(path, F1));
  free(path);

  path = pop_from_linq(linq, 0, &is_empty, &error_code);
  assert(!is_empty);
  assert(!error_code);
  assert(!strcmp(path, F2));
  free(path);

  free_linq(linq);
}
