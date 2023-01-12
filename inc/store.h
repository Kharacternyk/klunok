#include <stdbool.h>
#include <sys/types.h>

struct store;

struct store *create_store(const char *root, int *error_code);
void copy_to_store(const char *path, const char *version,
                   const struct store *store, int *error_code,
                   int *cleanup_error_code, bool *is_not_found);
void free_store(struct store *store);
