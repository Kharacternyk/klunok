#include "trace.h"
#include <stdbool.h>
#include <stddef.h>

struct set;

struct set *create_set(size_t size_guess, struct trace *trace);
bool is_in_set(const char *value, const struct set *set);
bool is_unique_within_set(const char *value, const struct set *set);
void add_to_set(const char *value, struct set *set, struct trace *trace);
void remove_from_set(const char *value, struct set *set, struct trace *trace);
void free_set(struct set *set);
