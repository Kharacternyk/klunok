#include <stdbool.h>

struct set;

struct set *create_set();
bool is_in_set(const char *value, struct set *set);
void add_to_set(const char *value, struct set *set);
