#include "bitmap.h"
#include <assert.h>

int main() {
  int error_code = 0;
  struct bitmap *bitmap = create_bitmap(0, &error_code);

  assert(!error_code);
  assert(!get_bit_in_bitmap(42, bitmap));

  set_bit_in_bitmap(42, bitmap, &error_code);
  assert(!error_code);
  assert(get_bit_in_bitmap(42, bitmap));
  assert(!get_bit_in_bitmap(2048, bitmap));

  set_bit_in_bitmap(2048, bitmap, &error_code);
  assert(!error_code);
  assert(get_bit_in_bitmap(42, bitmap));
  assert(get_bit_in_bitmap(2048, bitmap));
}
