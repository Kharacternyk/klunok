#include "store.h"
#include "messages.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define STORE_ROOT TEST_ROOT "/store"
#define COPYIED_FILE TEST_ROOT "/meson.build"
#define MISSING_FILE TEST_ROOT "/foobar"
#define FILE_VERSION "v123"
#define FILE_COPY STORE_ROOT COPYIED_FILE "/" FILE_VERSION

int main() {
  struct trace *trace = create_trace();
  struct store *store = create_store(STORE_ROOT, trace);
  assert(ok(trace));

  unlink(FILE_COPY);
  assert(access(FILE_COPY, F_OK));

  copy_to_store(COPYIED_FILE, FILE_VERSION, store, trace);
  assert(ok(trace));

  assert(!access(FILE_COPY, F_OK));
  unlink(FILE_COPY);

  copy_to_store(MISSING_FILE, FILE_VERSION, store, trace);
  assert(!ok(trace));
  assert(get_trace_message(trace) == messages.store.copy.file_does_not_exist);
  clear(trace);

  free(trace);
  free_store(store);
}
