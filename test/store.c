#include "../src/store.h"
#include <assert.h>

#define TEST_STORE "./store"
#define EXISTING_FILE "file"
#define MISSING_FILE "missing"

int main() {
  struct store *store = create_store(TEST_STORE);
  assert(is_in_store(EXISTING_FILE, store));
  assert(!is_in_store(MISSING_FILE, store));
}
