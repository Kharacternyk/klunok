#include "store.h"
#include "messages.h"
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#define STORE_ROOT "./store"
#define COPYIED_FILE TEST_ROOT "/meson.build"
#define MISSING_FILE TEST_ROOT "/foobar"
#define FILE_VERSION "v123"
#define FILE_COPY STORE_ROOT COPYIED_FILE "/" FILE_VERSION

void test_store() {
  struct trace *trace = create_trace();

  assert(access(FILE_COPY, F_OK) != 0);

  copy_to_store(COPYIED_FILE, FILE_VERSION, STORE_ROOT, trace);
  assert(ok(trace));

  assert(!access(FILE_COPY, F_OK));

  copy_to_store(MISSING_FILE, FILE_VERSION, STORE_ROOT, trace);
  assert(catch_static(messages.store.copy.file_does_not_exist, trace));

  free(trace);
}
