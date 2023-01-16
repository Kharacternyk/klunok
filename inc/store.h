#include "trace.h"
#include <stdbool.h>

struct store;

struct store *create_store(const char *root, struct trace *trace);
void copy_to_store(const char *path, const char *version,
                   const struct store *store, struct trace *trace);
void free_store(struct store *store);
