#include "../src/bitmap.h"
#include <assert.h>

void error_callback_function(void *parameter) { assert(parameter); }

int main() {
  struct callback *error_callback =
      create_callback(error_callback_function, NULL, NULL);

  struct bitmap *bitmap = create_bitmap(0, error_callback);
  assert(!get_bit_in_bitmap(42, bitmap));
  set_bit_in_bitmap(42, bitmap, error_callback);
  assert(get_bit_in_bitmap(42, bitmap));
  assert(!get_bit_in_bitmap(2048, bitmap));
  set_bit_in_bitmap(2048, bitmap, error_callback);
  assert(get_bit_in_bitmap(42, bitmap));
  assert(get_bit_in_bitmap(2048, bitmap));
}
