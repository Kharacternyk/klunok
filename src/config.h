#include "set.h"

struct config;

struct config *load_config(const char *path, struct callback *error_callback,
                           const char **error_message);
void free_config(struct config *config);
struct set *get_editors_from_config(struct config *config);
