#include "../src/set.h"
#include <assert.h>
#include <string.h>

#define S1 "test"
#define S2 "/home/nazar"

int main() {
  struct set *set = create_set();

  assert(!is_in_set(strdup(S1), set));
  add_to_set(S1, set);
  assert(is_in_set(strdup(S1), set));

  assert(!is_in_set(strdup(S2), set));
  add_to_set(S2, set);
  assert(is_in_set(strdup(S2), set));
  assert(is_in_set(strdup(S1), set));
}
