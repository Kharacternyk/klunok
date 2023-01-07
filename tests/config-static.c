#include "config.h"
#include <assert.h>

void check_default_config(struct config *config);

int main() {
  int error_code = 0;
  char *error_message = NULL;
  struct config *config = load_config("", &error_code, &error_message);

  assert(!error_code);
  assert(!error_message);

  check_default_config(config);
}
