struct trace;

struct params *parse_params(int argc, const char **argv, struct trace *trace);
const char *get_config_path(const struct params *params);
const char *get_privilege_dropping_path(const struct params *params);
const struct set *get_ignored_write_mounts(const struct params *params);
const struct set *get_ignored_exec_mounts(const struct params *params);
void free_params(struct params *params);
