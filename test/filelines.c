#include "../src/filelines.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void error_callback_function(void *parameter) { assert(parameter); }

int main() {
  struct callback *error_callback =
      create_callback(error_callback_function, NULL, NULL);
  struct set *editors =
      get_lines(PROJECT_ROOT "/test/editors", 12, error_callback);
  assert(is_in_set("vi", editors));
  assert(is_in_set("vim", editors));
  assert(is_in_set("nvim", editors));
  assert(is_in_set("rstudio", editors));
  assert(!is_in_set("qutebrowser", editors));
}
