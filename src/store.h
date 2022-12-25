#include "callback.h"
#include <stdbool.h>

struct store;

struct store *create_store(const char *root, struct callback *error_callback);
bool is_in_store(const char *path, const char *version, struct store *store,
                 struct callback *error_callback);
void link_to_store(const char *path, const char *version, struct store *store,
                   struct callback *error_callback);
void copy_to_store(const char *path, const char *version, struct store *store,
                   struct callback *error_callback);
