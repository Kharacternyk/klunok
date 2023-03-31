#include <stdbool.h>
#include <stddef.h>

struct trace;

struct bitmap *create_bitmap(size_t size_guess, struct trace *trace);
void set_bit(size_t bit, struct bitmap *bitmap, struct trace *trace);
void unset_bit(size_t bit, struct bitmap *bitmap);
bool get_bit(size_t bit, const struct bitmap *bitmap);
void free_bitmap(struct bitmap *bitmap);
