#include "timestamp.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void error_callback_function(void *parameter) { assert(parameter); }

int main() {
  struct callback *error_callback =
      create_callback(error_callback_function, NULL, NULL);

  assert(!strcmp(get_timestamp("abc", 7, error_callback), "abc"));
  assert(!strcmp(get_timestamp("abc", 3, error_callback), "abc"));
  assert(strlen(get_timestamp("%C", 2, error_callback)) == 2);
  assert(strlen(get_timestamp("%Y%m%d%H%M%S", 14, error_callback)) == 14);
}
