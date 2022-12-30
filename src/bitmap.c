#include "bitmap.h"
#include <stdlib.h>
#include <string.h>

struct bitmap {
  size_t size;
  bool *array;
};

struct bitmap *create_bitmap(size_t size_guess,
                             struct callback *error_callback) {
  struct bitmap *bitmap = malloc(sizeof(struct bitmap));
  if (!bitmap) {
    invoke_callback(error_callback);
    return NULL;
  }

  bool *array = calloc(size_guess, sizeof(bool));
  if (!array) {
    invoke_callback(error_callback);
    free(bitmap);
    return NULL;
  }

  bitmap->array = array;
  bitmap->size = size_guess;

  return bitmap;
}

void set_bit_in_bitmap(size_t bit, struct bitmap *bitmap,
                       struct callback *error_callback) {
  if (bit >= bitmap->size) {
    size_t new_size = bit * 2;
    bool *new_array = calloc(new_size, sizeof(bool));
    if (!new_array) {
      return invoke_callback(error_callback);
    }
    memcpy(new_array, bitmap->array, bitmap->size);
    free(bitmap->array);
    bitmap->array = new_array;
    bitmap->size = new_size;
  }

  bitmap->array[bit] = true;
}

void unset_bit_in_bitmap(size_t bit, struct bitmap *bitmap) {
  if (bit < bitmap->size) {
    bitmap->array[bit] = false;
  }
}

bool get_bit_in_bitmap(size_t bit, struct bitmap *bitmap) {
  if (bit < bitmap->size) {
    return bitmap->array[bit];
  }
  return false;
}