#include "timestamp.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void error_callback_function(void *parameter) { assert(parameter); }

int main() {
  struct callback *error_callback =
      create_callback(error_callback_function, NULL, NULL);
  bool is_overflow = false;

  assert(
      !strcmp(get_timestamp("abc", 7, error_callback, &is_overflow), "abc") &&
      !is_overflow);
  assert(
      !strcmp(get_timestamp("abc", 3, error_callback, &is_overflow), "abc") &&
      !is_overflow);
  assert(strlen(get_timestamp("%C", 2, error_callback, &is_overflow)) == 2 &&
         !is_overflow);
  assert(strlen(get_timestamp("%Y%m%d%H%M%S", 14, error_callback,
                              &is_overflow)) == 14 &&
         !is_overflow);

  get_timestamp("%Y", 3, NULL, &is_overflow);
  assert(is_overflow);
}
