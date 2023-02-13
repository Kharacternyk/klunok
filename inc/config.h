#include "set.h"
#include "trace.h"
#include <fcntl.h>

struct config;

struct config *load_config(const char *path, struct trace *trace);

const struct set *get_editors(const struct config *config);
const char *get_store_root(const struct config *config);
const char *get_queue_path(const struct config *config);
const char *get_version_pattern(const struct config *config);
size_t get_debounce_seconds(const struct config *config);
size_t get_path_length_guess(const struct config *config);
size_t get_elf_interpreter_count_guess(const struct config *config);
size_t get_executable_count_guess(const struct config *config);
size_t get_queue_size_guess(const struct config *config);
pid_t get_max_pid_guess(const struct config *config);

void free_config(struct config *config);
