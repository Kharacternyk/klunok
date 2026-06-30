#include "serialize.h"

void write_u64(uint64_t value, uint8_t *destination) {
  for (uint8_t i = sizeof value; i; --i) {
    destination[i - 1] = value;
    value >>= 8;
  }
}
