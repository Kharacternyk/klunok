#include "timestamp.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main() {
  int error_code = 0;
  bool is_overflow = false;
  const char *timestamp = get_timestamp("abc", 7, &error_code, &is_overflow);
  assert(!error_code);
  assert(!is_overflow);
  assert(!strcmp(timestamp, "abc"));

  timestamp = get_timestamp("abc", 3, &error_code, &is_overflow);
  assert(!error_code);
  assert(!is_overflow);
  assert(!strcmp(timestamp, "abc"));

  timestamp = get_timestamp("%C", 2, &error_code, &is_overflow);
  assert(!error_code);
  assert(!is_overflow);
  assert(strlen(timestamp) == 2);

  timestamp = get_timestamp("%Y%m%d%H%M%S", 14, &error_code, &is_overflow);
  assert(!error_code);
  assert(!is_overflow);
  assert(strlen(timestamp) == 14);

  get_timestamp("%Y", 3, &error_code, &is_overflow);
  assert(!error_code);
  assert(is_overflow);
}
