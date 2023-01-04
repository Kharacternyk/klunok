#include "set.h"

struct config;

struct config *load_config(const char *path, int *error_code,
                           const char **static_error_message,
                           char **dynamic_error_message);
const struct set *get_configured_editors(const struct config *config);
const char *get_configured_version_pattern(const struct config *config);
void free_config(struct config *config);
