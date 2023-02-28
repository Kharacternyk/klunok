#include <stdbool.h>
#include <stddef.h>

struct set;
struct trace;

struct set *create_set(size_t size_guess, struct trace *trace);

bool is_in_set(const char *value, const struct set *set);
void remove_from_set(const char *value, struct set *set);
void add_to_set(const char *value, struct set *set, struct trace *trace);

size_t get_count_in_set(const char *value, const struct set *set);
void set_count_in_set(size_t count, const char *value, struct set *set,
                      struct trace *trace);

size_t get_best_match_count_in_set(const char *value, char separator,
                                   const struct set *set);

void free_set(struct set *set);
