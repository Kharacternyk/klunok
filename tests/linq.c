#include "linq.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define INSTANT_LINQ_PATH TEST_ROOT "/linq/instant"
#define LAGGED_LINQ_PATH TEST_ROOT "/linq/lagged"
#define F1 TEST_ROOT "/meson.build"
#define F2 TEST_ROOT "/linq.c"

int main() {
  int error_code = 0;
  struct linq *linq = load_linq(INSTANT_LINQ_PATH, 0, &error_code);
  assert(!error_code);

  time_t retry_after_seconds = 0;
  char *path = pop_from_linq(linq, 0, &retry_after_seconds, &error_code);
  assert(!error_code);
  assert(retry_after_seconds < 0);
  retry_after_seconds = 0;

  push_to_linq(F1, linq, &error_code);
  assert(!error_code);
  path = pop_from_linq(linq, 0, &retry_after_seconds, &error_code);
  assert(!retry_after_seconds);
  assert(!error_code);
  assert(!strcmp(path, F1));

  free(path);

  push_to_linq(F1, linq, &error_code);
  assert(!error_code);
  push_to_linq(F2, linq, &error_code);
  assert(!error_code);

  path = pop_from_linq(linq, 0, &retry_after_seconds, &error_code);
  assert(!retry_after_seconds);
  assert(!error_code);
  assert(!strcmp(path, F1));
  free(path);

  path = pop_from_linq(linq, 0, &retry_after_seconds, &error_code);
  assert(!retry_after_seconds);
  assert(!error_code);
  assert(!strcmp(path, F2));
  free(path);

  free_linq(linq);

  unlink(LAGGED_LINQ_PATH "/0");
  linq = load_linq(LAGGED_LINQ_PATH, 3600, &error_code);
  push_to_linq(F1, linq, &error_code);
  assert(!error_code);
  path = pop_from_linq(linq, 0, &retry_after_seconds, &error_code);
  assert(!error_code);
  assert(retry_after_seconds > 0);
  unlink(LAGGED_LINQ_PATH "/0");

  free_linq(linq);
}
