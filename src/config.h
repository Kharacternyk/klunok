#include "set.h"

struct config;

struct config *load_config(const char *path,
                           const struct callback *error_callback,
                           const char **error_message);
const struct set *get_configured_editors(const struct config *config);
const char *get_configured_version_pattern(const struct config *config);
void free_config(struct config *config);
