#include "store.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#define STORE_ROOT TEST_ROOT "/store"
#define COPYIED_FILE TEST_ROOT "/meson.build"
#define FILE_VERSION "v123"
#define FILE_COPY STORE_ROOT COPYIED_FILE "/" FILE_VERSION

void typed_error_callback_function(const char **message) {
  perror(*message);
  assert(false);
}

void error_callback_function(void *message) {
  typed_error_callback_function(message);
}

int main() {
  const char *message = "create_store";
  struct callback *error_callback =
      create_callback(error_callback_function, &message, NULL);

  struct store *store = create_store(STORE_ROOT, error_callback);

  unlink(FILE_COPY);
  assert(access(FILE_COPY, F_OK));

  message = "copy_to_store";
  copy_to_store(COPYIED_FILE, FILE_VERSION, store, error_callback);

  assert(!access(FILE_COPY, F_OK));
  unlink(FILE_COPY);
}
