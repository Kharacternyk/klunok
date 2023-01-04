#include "sys/types.h"
#include <stdbool.h>

struct store;

struct store *create_store(const char *root, int *error_code);
void copy_to_store(const char *path, const char *version,
                   const struct store *store, int *error_code,
                   int *cleanup_error_code);
uid_t get_store_uid(const struct store *store);
gid_t get_store_gid(const struct store *store);
