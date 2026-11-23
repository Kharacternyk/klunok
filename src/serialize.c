#include "serialize.h"

static size_t write(uint64_t value, uint8_t *destination, size_t size) {
  for (uint8_t i = size; i; --i) {
    destination[i - 1] = value;
    value >>= 8;
  }
  return size;
}

size_t write_u64(uint64_t value, uint8_t *destination) {
  return write(value, destination, sizeof value);
}

size_t write_u32(uint32_t value, uint8_t *destination) {
  return write(value, destination, sizeof value);
}
