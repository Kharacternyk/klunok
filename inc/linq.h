#include "trace.h"
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

struct linq;

struct linq *load_linq(const char *path, time_t debounce_seconds,
                       size_t entry_length_guess, struct trace *trace);
void push_to_linq(const char *path, struct linq *linq, struct trace *trace);
char *get_linq_head(struct linq *linq, time_t *retry_after_seconds,
                    struct trace *trace);
void pop_from_linq(struct linq *linq, struct trace *trace);
void free_linq(struct linq *linq);
