#include "storepath.h"
#include "trace.h"
#include <assert.h>
#include <string.h>

#define ROOT "/abcd/root/efg"
#define EXTENSION ".build"
#define PATH "src/klunok/meson" EXTENSION
#define VERSION "v12-34-56"

void test_storepath(struct trace *trace) {
  struct store_path *store_path = create_store_path(ROOT, PATH, VERSION, trace);
  assert(ok(trace));
  assert(!strcmp(get_current_path(store_path),
                 ROOT "/" PATH "/" VERSION EXTENSION));

  increment(store_path, trace);
  assert(ok(trace));
  assert(!strcmp(get_current_path(store_path),
                 ROOT "/" PATH "/" VERSION "-1" EXTENSION));

  increment(store_path, trace);
  assert(ok(trace));
  assert(!strcmp(get_current_path(store_path),
                 ROOT "/" PATH "/" VERSION "-2" EXTENSION));

  free_store_path(store_path);
}
