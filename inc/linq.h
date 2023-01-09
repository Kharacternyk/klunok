#include <stdbool.h>
#include <stddef.h>

struct linq;

struct linq *load_linq(const char *path, int *error_code, bool *is_corrupted);
void push_to_linq(const char *path, struct linq *linq, int *error_code);
char *pop_from_linq(struct linq *linq, size_t length_guess, bool *is_empty,
                    int *error_code);
void free_linq(struct linq *linq);
