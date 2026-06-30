#include "serialize.h"
#include "trace.h"
#include <assert.h>

void test_serialize(struct trace *trace) {
  uint8_t destination[10];

  for (uint8_t i = 0; i < sizeof destination / sizeof destination[0]; ++i) {
    destination[i] = i;
  }

  write_u64(3 + 256, destination);

  for (uint8_t i = 0; i < sizeof destination / sizeof destination[0]; ++i) {
    uint8_t expected = i;

    if (i == sizeof(uint64_t) - 1) {
      expected = 3;
    } else if (i == sizeof(uint64_t) - 2) {
      expected = 1;
    } else if (i < sizeof(uint64_t)) {
      expected = 0;
    }

    assert(destination[i] == expected);
  }
}
