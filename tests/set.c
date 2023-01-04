#include "set.h"
#include <assert.h>
#include <string.h>

#define S1 "/home/nazar"
#define S2 "keynumber1"
#define S3 "yerkn11ke1" /* hash collision with S2 */

int main() {
  int error_code = 0;
  struct set *set = create_set(0, &error_code);
  assert(!error_code);

  assert(!is_in_set(S1, set));
  add_to_set(S1, set, &error_code);
  assert(!error_code);
  assert(is_in_set(S1, set));

  assert(!is_in_set(S2, set));
  add_to_set(S2, set, &error_code);
  assert(!error_code);
  assert(is_in_set(S2, set));
  assert(is_in_set(S1, set));

  assert(!is_in_set(S3, set));
  add_to_set(S3, set, &error_code);
  assert(!error_code);
  assert(is_in_set(S3, set));
  assert(is_in_set(S2, set));
  assert(is_in_set(S1, set));
}
