#include "set.h"

struct config;

struct config *load_config(const char *path, struct callback *error_callback,
                           const char **error_message);
struct set *get_configured_editors(struct config *config);
const char *get_configured_version_pattern(struct config *config);
void free_config(struct config *config);
