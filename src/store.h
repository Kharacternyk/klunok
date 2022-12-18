#include <stdbool.h>

struct store;

struct store *create_store(const char *root);
bool is_in_store(const char *path, struct store *store);
bool link_to_store(const char *path, struct store *store);
