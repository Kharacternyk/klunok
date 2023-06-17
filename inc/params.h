#include <stdbool.h>

struct trace;

struct params *parse_params(int argc, const char **argv, struct trace *trace)
    __attribute__((warn_unused_result));
bool is_help_requested(const struct params *params)
    __attribute__((pure, nonnull, warn_unused_result));
bool is_version_requested(const struct params *params)
    __attribute__((pure, nonnull, warn_unused_result));
const char *get_config_path(const struct params *params)
    __attribute__((pure, returns_nonnull, nonnull, warn_unused_result));
const char *get_privilege_dropping_path(const struct params *params)
    __attribute__((pure, returns_nonnull, nonnull, warn_unused_result));
const struct list *get_write_mounts(const struct params *params)
    __attribute__((pure, returns_nonnull, nonnull, warn_unused_result));
const struct list *get_exec_mounts(const struct params *params)
    __attribute__((pure, returns_nonnull, nonnull, warn_unused_result));
void free_params(struct params *params);
