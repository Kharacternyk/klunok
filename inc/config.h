#include "set.h"
#include "trace.h"
#include <fcntl.h>

struct config;

struct config *load_config(const char *path, struct trace *trace);

const struct set *get_configured_editors(const struct config *config);
const char *get_configured_store_root(const struct config *config);
const char *get_configured_queue_path(const struct config *config);
const char *get_configured_version_pattern(const struct config *config);
const char *get_configured_ld_linux_glob(const struct config *config);
size_t get_configured_debounce_seconds(const struct config *config);
size_t get_configured_version_max_length(const struct config *config);
size_t get_configured_path_length_guess(const struct config *config);
size_t get_configured_elf_interpreter_count_guess(const struct config *config);
size_t get_configured_executable_count_guess(const struct config *config);
size_t get_configured_queue_size_guess(const struct config *config);
pid_t get_configured_max_pid_guess(const struct config *config);

void free_config(struct config *config);
