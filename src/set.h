#include "callback.h"
#include <stdbool.h>
#include <stddef.h>

struct set;

struct set *create_set(size_t size_guess,
                       const struct callback *error_callback);
bool is_in_set(const char *value, const struct set *set);
void add_to_set(const char *value, struct set *set,
                const struct callback *error_callback);
void free_set(struct set *set);
