#include "sieve.h"
#include "set.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>

void test_sieve(struct trace *trace) {
  struct set *sets[] = {
      create_set(0, trace),
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

  add("/home", sets[3], trace);
  add("nazar", sets[3], trace);
  add("nazar/src", sets[3], trace);
  add("klunok", sets[3], trace);

  assert(ok(trace));

  const char *path = "/home/nazar/src/klunok/tests/sieve.c";

  struct sieved_path *sieved_path =
      sieve(path, 6, (const struct set **)sets, 4, trace);
  assert(ok(trace));

  assert(!get_hiding_dot(sieved_path));
  assert(get_sieved_ends(sieved_path)[0] == path + 22);
  assert(get_sieved_ends(sieved_path)[1] == path + 1);
  assert(get_sieved_ends(sieved_path)[2] == NULL);
  assert(get_sieved_ends(sieved_path)[3] == path + 15);
  free_sieved_path(sieved_path);

  path = "/home/nazar/.config/pacwall/pacwall.conf";
  sieved_path = sieve(path, 1, NULL, 0, trace);
  assert(ok(trace));
  assert(get_hiding_dot(sieved_path) == path + 12);
  free_sieved_path(sieved_path);

  for (size_t i = 0; i < 4; ++i) {
    free_set(sets[i]);
  }
}
