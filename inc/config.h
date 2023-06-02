#include <fcntl.h>
#include <stddef.h>

struct trace;

struct config *load_config(const char *path, struct trace *trace)
    __attribute__((warn_unused_result));

const struct set *get_editors(const struct config *config)
    __attribute__((pure, warn_unused_result));
const struct set *get_history_paths(const struct config *config)
    __attribute__((pure, warn_unused_result));
const struct set *get_excluded_paths(const struct config *config)
    __attribute__((pure, warn_unused_result));
const struct set *get_included_paths(const struct config *config)
    __attribute__((pure, warn_unused_result));
const char *get_store_root(const struct config *config)
    __attribute__((pure, warn_unused_result));
const char *get_queue_path(const struct config *config)
    __attribute__((pure, warn_unused_result));
const char *get_journal_path(const struct config *config)
    __attribute__((pure, warn_unused_result));
const char *get_offset_store_root(const struct config *config)
    __attribute__((pure, warn_unused_result));
const char *get_journal_timestamp_pattern(const struct config *config)
    __attribute__((pure, warn_unused_result));
const char *get_version_pattern(const struct config *config)
    __attribute__((pure, warn_unused_result));
size_t get_debounce_seconds(const struct config *config)
    __attribute__((pure, warn_unused_result));
size_t get_path_length_guess(const struct config *config)
    __attribute__((pure, warn_unused_result));
size_t get_elf_interpreter_count_guess(const struct config *config)
    __attribute__((pure, warn_unused_result));
size_t get_queue_size_guess(const struct config *config)
    __attribute__((pure, warn_unused_result));
pid_t get_max_pid_guess(const struct config *config)
    __attribute__((pure, warn_unused_result));
const char *get_event_open_exec_not_editor(const struct config *config)
    __attribute__((pure, warn_unused_result));
const char *get_event_open_exec_editor(const struct config *config)
    __attribute__((pure, warn_unused_result));
const char *get_event_close_write_not_by_editor(const struct config *config)
    __attribute__((pure, warn_unused_result));
const char *get_event_close_write_by_editor(const struct config *config)
    __attribute__((pure, warn_unused_result));
const char *get_event_queue_head_deleted(const struct config *config)
    __attribute__((pure, warn_unused_result));
const char *get_event_queue_head_forbidden(const struct config *config)
    __attribute__((pure, warn_unused_result));
const char *get_event_queue_head_stored(const struct config *config)
    __attribute__((pure, warn_unused_result));

void free_config(struct config *config);
