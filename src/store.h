#include "callback.h"
#include "sys/types.h"
#include <stdbool.h>

struct store;

struct store *create_store(const char *root,
                           const struct callback *error_callback);
void copy_to_store(const char *path, const char *version,
                   const struct store *store,
                   const struct callback *error_callback);
uid_t get_store_uid(const struct store *store);
gid_t get_store_gid(const struct store *store);
