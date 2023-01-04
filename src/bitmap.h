#include <stdbool.h>
#include <stddef.h>

struct bitmap;

struct bitmap *create_bitmap(size_t size_guess, int *error_code);
void set_bit_in_bitmap(size_t bit, struct bitmap *bitmap, int *error_code);
void unset_bit_in_bitmap(size_t bit, struct bitmap *bitmap);
bool get_bit_in_bitmap(size_t bit, const struct bitmap *bitmap);
