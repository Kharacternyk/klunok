#include "../src/store.h"
#include <assert.h>
#include <unistd.h>

#define TEST_STORE PROJECT_ROOT "/test/store"
#define EXISTING_FILE "/file"
#define MISSING_FILE "/missing"
#define CREATED_FILE PROJECT_ROOT "/meson.build"

int main() {
  struct store *store = create_store(TEST_STORE);
  assert(is_in_store(EXISTING_FILE, store));
  assert(!is_in_store(MISSING_FILE, store));

  unlink(TEST_STORE CREATED_FILE);
  assert(!is_in_store(CREATED_FILE, store));
  assert(link_to_store(CREATED_FILE, store));
  assert(is_in_store(CREATED_FILE, store));
  unlink(TEST_STORE CREATED_FILE);
  assert(!is_in_store(CREATED_FILE, store));
}
