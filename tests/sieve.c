#include "sieve.h"
#include "set.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>

void test_sieve() {
  struct trace *trace = create_trace();

  struct set *sets[] = {
      create_set(0, trace),
      create_set(0, trace),
      create_set(0, trace),
  };
  add("/home/nazar", sets[0], trace);
  add("/home/nazar/src/klunok", sets[0], trace);
  add("/home/nazar/src/klunok/src", sets[0], trace);
  add("/", sets[1], trace);
  add("/etc", sets[1], trace);
  add("/usr/bin", sets[2], trace);
  add("/tmp", sets[2], trace);
  assert(ok(trace));

  const char *path = "/home/nazar/src/klunok/tests/sieve.c";

  struct sieved_path *sieved_path =
      sieve(path, 3, (const struct set **)sets, trace);
  assert(ok(trace));

  assert(!is_hidden(sieved_path));
  assert(get_sieved_ends(sieved_path)[0] == path + 22);
  assert(get_sieved_ends(sieved_path)[1] == path + 1);
  assert(get_sieved_ends(sieved_path)[2] == NULL);
  free_sieved_path(sieved_path);

  sieved_path =
      sieve("/home/nazar/.config/pacwall/pacwall.conf", 0, NULL, trace);
  assert(ok(trace));
  assert(is_hidden(sieved_path));
  free_sieved_path(sieved_path);

  for (size_t i = 0; i < 3; ++i) {
    free_set(sets[i]);
  }
  free(trace);
}
