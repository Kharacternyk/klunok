#include <stdbool.h>
#include <stddef.h>

struct trace;

struct bitmap *create_bitmap(size_t size_guess, struct trace *trace)
    __attribute__((warn_unused_result));
void set_bit(size_t bit, struct bitmap *bitmap, struct trace *trace);
void unset_bit(size_t bit, struct bitmap *bitmap) __attribute__((nonnull));
bool get_bit(size_t bit, const struct bitmap *bitmap)
    __attribute__((pure, nonnull, warn_unused_result));
void free_bitmap(struct bitmap *bitmap);
