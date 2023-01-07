#include "store.h"
#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#define STORE_ROOT TEST_ROOT "/store"
#define COPYIED_FILE TEST_ROOT "/meson.build"
#define FILE_VERSION "v123"
#define FILE_COPY STORE_ROOT COPYIED_FILE "/" FILE_VERSION
#define NEW_STORE_ROOT TEST_ROOT "/tmp-store"

int main() {
  int error_code = 0;
  struct store *store = create_store(STORE_ROOT, &error_code);
  assert(!error_code);

  unlink(FILE_COPY);
  assert(access(FILE_COPY, F_OK));

  int rollback_error_code = 0;
  copy_to_store(COPYIED_FILE, FILE_VERSION, store, &error_code,
                &rollback_error_code);
  assert(!error_code);
  assert(!rollback_error_code);

  assert(!access(FILE_COPY, F_OK));
  unlink(FILE_COPY);

  rmdir(NEW_STORE_ROOT);
  assert(access(FILE_COPY, F_OK));

  mkdir(NEW_STORE_ROOT, S_IRWXU);
  assert(!access(NEW_STORE_ROOT, F_OK));

  free_store(store);
}
