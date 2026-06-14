#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct buffer_view;
struct trace;

struct set *create_set(size_t size_guess, struct trace *trace)
    __attribute__((warn_unused_result));

bool is_empty(const struct set *set)
    __attribute__((pure, nonnull, warn_unused_result));

void add(const char *value, struct set *set, struct trace *trace);
void add_with_metadata(const char *value, uint64_t metadata, struct set *set,
                       struct trace *trace);

bool is_within(const struct buffer_view *value, const struct set *set)
    __attribute__((pure, nonnull, warn_unused_result));
void pop(const struct buffer_view *value, struct set *set)
    __attribute__((nonnull));
uint32_t get_count(const struct buffer_view *value, const struct set *set)
    __attribute__((pure, nonnull, warn_unused_result));
uint64_t get_last_metadata(const struct buffer_view *value,
                           const struct set *set)
    __attribute__((pure, nonnull, warn_unused_result));

void free_set(struct set *set);
