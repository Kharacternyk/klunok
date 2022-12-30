#include "callback.h"
#include <stdbool.h>
#include <stddef.h>

struct bitmap;

struct bitmap *create_bitmap(size_t size_guess,
                             struct callback *error_callback);
void set_bit_in_bitmap(size_t bit, struct bitmap *bitmap,
                       struct callback *error_callback);
void unset_bit_in_bitmap(size_t bit, struct bitmap *bitmap);
bool get_bit_in_bitmap(size_t bit, struct bitmap *bitmap);
