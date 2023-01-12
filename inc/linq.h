#include <stdbool.h>
#include <stddef.h>
#include <time.h>

struct linq;

struct linq *load_linq(const char *path, time_t debounce_seconds,
                       int *error_code);
void push_to_linq(const char *path, struct linq *linq, int *error_code);
char *pop_from_linq(struct linq *linq, size_t length_guess,
                    time_t *retry_after_seconds, int *error_code);
void free_linq(struct linq *linq);
