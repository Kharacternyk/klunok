#include "base16.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main() {
  bool is_malformed = false;
  size_t value = decode_from_base16("00000f", &is_malformed);
  assert(!is_malformed);
  assert(value == 15);

  value = decode_from_base16("00000a4", &is_malformed);
  assert(!is_malformed);
  assert(value == 164);

  decode_from_base16("metallica", &is_malformed);
  assert(is_malformed);
  is_malformed = false;

  int error_code = 0;
  char *string = encode_to_base16(1, &error_code);
  assert(strchr(string, '1'));
  free(string);

  for (size_t i = 0; i < 2048; ++i) {
    int error_code = 0;
    string = encode_to_base16(i, &error_code);
    assert(!error_code);
    value = decode_from_base16(string, &is_malformed);
    assert(!is_malformed);
    assert(value == i);
    free(string);
  }
}
