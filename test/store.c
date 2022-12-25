#include "../src/store.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#define STORE_ROOT PROJECT_ROOT "/test/store"
#define FILE_VERSION "test"
#define EXISTING_FILE "/file"
#define MISSING_FILE "/missing"
#define LINKED_FILE PROJECT_ROOT "/meson.build"
#define COPYIED_FILE PROJECT_ROOT "/LICENSE"

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
      create_callback(error_callback_function, &message);

  struct store *store = create_store(STORE_ROOT, error_callback);
  message = "is_in_store existing";
  assert(is_in_store(EXISTING_FILE, FILE_VERSION, store, error_callback));
  message = "is_in_store missing";
  assert(!is_in_store(MISSING_FILE, FILE_VERSION, store, error_callback));

  unlink(STORE_ROOT LINKED_FILE "/" FILE_VERSION);
  message = "is_in_store linked 1";
  assert(!is_in_store(LINKED_FILE, FILE_VERSION, store, error_callback));

  message = "link_to_store";
  link_to_store(LINKED_FILE, FILE_VERSION, store, error_callback);

  message = "is_in_store linked 2";
  assert(is_in_store(LINKED_FILE, FILE_VERSION, store, error_callback));
  unlink(STORE_ROOT LINKED_FILE "/" FILE_VERSION);
  message = "is_in_store linked 3";
  assert(!is_in_store(LINKED_FILE, FILE_VERSION, store, error_callback));

  unlink(STORE_ROOT COPYIED_FILE "/" FILE_VERSION);
  message = "is_in_store copied 1";
  assert(!is_in_store(COPYIED_FILE, FILE_VERSION, store, error_callback));

  message = "copy_to_store";
  copy_to_store(COPYIED_FILE, FILE_VERSION, store, error_callback);

  message = "is_in_store copied 2";
  assert(is_in_store(COPYIED_FILE, FILE_VERSION, store, error_callback));
  unlink(STORE_ROOT COPYIED_FILE "/" FILE_VERSION);
  message = "is_in_store copied 3";
  assert(!is_in_store(COPYIED_FILE, FILE_VERSION, store, error_callback));
}
