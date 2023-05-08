#include "bitmap.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>

void test_bitmap(struct trace *trace) {
  struct bitmap *bitmap = create_bitmap(0, trace);

  assert(ok(trace));
  assert(!get_bit(42, bitmap));

  set_bit(42, bitmap, trace);
  assert(ok(trace));
  assert(get_bit(42, bitmap));
  assert(!get_bit(2048, bitmap));

  set_bit(2048, bitmap, trace);
  assert(ok(trace));
  assert(get_bit(42, bitmap));
  assert(get_bit(2048, bitmap));

  free_bitmap(bitmap);
}
