#include "store.h"
#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#define STORE_ROOT TEST_ROOT "/store"
#define COPYIED_FILE TEST_ROOT "/meson.build"
#define MISSING_FILE TEST_ROOT "/foobar"
#define FILE_VERSION "v123"
#define FILE_COPY STORE_ROOT COPYIED_FILE "/" FILE_VERSION

int main() {
  int error_code = 0;
  struct store *store = create_store(STORE_ROOT, &error_code);
  assert(!error_code);

  unlink(FILE_COPY);
  assert(access(FILE_COPY, F_OK));

  bool is_not_found = false;
  int rollback_error_code = 0;
  copy_to_store(COPYIED_FILE, FILE_VERSION, store, &error_code,
                &rollback_error_code, &is_not_found);
  assert(!error_code);
  assert(!rollback_error_code);
  assert(!is_not_found);

  assert(!access(FILE_COPY, F_OK));
  unlink(FILE_COPY);

  copy_to_store(MISSING_FILE, FILE_VERSION, store, &error_code,
                &rollback_error_code, &is_not_found);
  assert(!error_code);
  assert(!rollback_error_code);
  assert(is_not_found);

  free_store(store);
}
