#include "bitmap.h"
#include "trace.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

struct bitmap {
  bool *array;
  size_t size;
};

struct bitmap *create_bitmap(size_t size_guess, struct trace *trace) {
  struct bitmap *bitmap = TNULL(malloc(sizeof(struct bitmap)), trace);
  bool *array = TNULL(calloc(size_guess, sizeof(bool)), trace);
  if (!ok(trace)) {
    free(bitmap);
    free(array);
    return NULL;
  }
  bitmap->array = array;
  bitmap->size = size_guess;
  return bitmap;
}

void set_bit(size_t bit, struct bitmap *bitmap, struct trace *trace) {
  if (!ok(trace)) {
    return;
  }

  if (bit >= bitmap->size) {
    size_t new_size = bit * 2;
    bool *new_array = TNULL(calloc(new_size, sizeof(bool)), trace);
    if (!ok(trace)) {
      return;
    }
    memcpy(new_array, bitmap->array, bitmap->size);
    free(bitmap->array);
    bitmap->array = new_array;
    bitmap->size = new_size;
  }

  bitmap->array[bit] = true;
}

void unset_bit(size_t bit, struct bitmap *bitmap) {
  if (bit < bitmap->size) {
    bitmap->array[bit] = false;
  }
}

bool get_bit(size_t bit, const struct bitmap *bitmap) {
  if (bit < bitmap->size) {
    return bitmap->array[bit];
  }
  return false;
}

void free_bitmap(struct bitmap *bitmap) {
  if (bitmap) {
    free(bitmap->array);
    free(bitmap);
  }
}
