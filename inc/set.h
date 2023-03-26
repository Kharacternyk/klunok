#include <stdbool.h>
#include <stddef.h>

struct set;
struct buffer_view;
struct trace;

struct set *create_set(size_t size_guess, struct trace *trace);

bool is_empty(const struct set *set);

void add_to_set(const char *value, struct set *set, struct trace *trace);

bool is_in_set(const struct buffer_view *value, const struct set *set);
void remove_from_set(const struct buffer_view *value, struct set *set);
size_t get_count_in_set(const struct buffer_view *value, const struct set *set);

void free_set(struct set *set);
