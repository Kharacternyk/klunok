#include "callback.h"
#include <stdbool.h>
#include <stddef.h>

struct set;

struct set *create_set(size_t size_guess, struct callback *error_callback);
bool is_in_set(const char *value, struct set *set);
void add_to_set(const char *value, struct set *set,
                struct callback *error_callback);
