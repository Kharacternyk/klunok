#include "config.h"
#include "set.h"
#include "trace.h"
#include <errno.h>
#include <messages.h>
#include <stdlib.h>

static const char *const store_root = "./klunok/store";
static const char *const queue_path = "./klunok/var/queue";
static const char *const journal_path = "./klunok/var/journal";
static const char *const journal_timestamp_pattern = "%Y-%m-%d-%H-%M";
static const char *const version_pattern = "v%Y-%m-%d-%H-%M";
static const char *const cursor_version = ".cursor";
static const size_t debounce_seconds = 60;
static const size_t path_length_guess = 1024;
static const pid_t max_pid_guess = 1 << 15;
static const size_t elf_interpreter_count_guess = 1;
static const size_t queue_size_guess = 2 * debounce_seconds;
static const char *const editors[] = {
    "atom",
    "code",
    "codium",
    "gedit",
    "gnome-text-editor",
    "howl",
    "hx",
    "inkscape",
    "kak",
    "kate",
    "kwrite",
    "micro",
    "nano",
    "notepadqq-bin",
    "nvim",
    "pluma",
    "rsession",
    "soffice.bin",
    "sublime_text",
    "vi",
    "vim",
    "xed",
    ".gedit-wrapped",
    ".gnome-text-editor-wrapped",
    ".howl-wrapped",
    ".hx-wrapped",
    ".inkscape-wrapped",
    ".kate-wrapped",
    ".kwrite-wrapped",
    ".pluma-wrapped",
    ".xed-wrapped",
};
static const char *const history_paths[] = {};
static const char *const excluded_paths[] = {};
static const char *const included_paths[] = {};
static const char *const event_open_exec_not_editor;
static const char *const event_open_exec_editor;
static const char *const event_close_write_not_by_editor;
static const char *const event_close_write_by_editor;
static const char *const event_queue_head_deleted;
static const char *const event_queue_head_forbidden;
static const char *const event_queue_head_stored = "";

struct config {
  struct set *editors;
  struct set *history_paths;
  struct set *excluded_paths;
  struct set *included_paths;
};

static struct set *load_set(const char *const *array, size_t size,
                            struct trace *trace) {
  struct set *set = create_set(size / sizeof(char *), trace);
  for (size_t i = 0; ok(trace) && i < size / sizeof(char *); ++i) {
    add(editors[i], set, trace);
  }
  return set;
}

struct config *load_config(const char *path, struct trace *trace) {
  if (ok(trace) && path) {
    throw_static(messages.config.is_static, trace);
    return NULL;
  }

  struct config *config = TNULL(calloc(1, sizeof(struct config)), trace);
  if (!ok(trace)) {
    return NULL;
  }

  config->editors = load_set(editors, sizeof editors, trace);
  config->history_paths = load_set(history_paths, sizeof history_paths, trace);
  config->excluded_paths =
      load_set(excluded_paths, sizeof excluded_paths, trace);
  config->included_paths =
      load_set(included_paths, sizeof included_paths, trace);

  if (!ok(trace)) {
    free_config(config);
    return NULL;
  }
  return config;
}

const struct set *get_editors(const struct config *config) {
  return config->editors;
}

const struct set *get_history_paths(const struct config *config) {
  return config->editors;
}

const struct set *get_excluded_paths(const struct config *config) {
  return config->excluded_paths;
}

const struct set *get_included_paths(const struct config *config) {
  return config->included_paths;
}

const char *get_store_root(const struct config *config) { return store_root; }

const char *get_queue_path(const struct config *config) { return queue_path; }

const char *get_journal_path(const struct config *config) {
  return journal_path;
}

const char *get_journal_timestamp_pattern(const struct config *config) {
  return journal_timestamp_pattern;
}

const char *get_version_pattern(const struct config *config) {
  return version_pattern;
}

const char *get_cursor_version(const struct config *config) {
  return cursor_version;
}

size_t get_debounce_seconds(const struct config *config) {
  return debounce_seconds;
}

size_t get_path_length_guess(const struct config *config) {
  return path_length_guess;
}

pid_t get_max_pid_guess(const struct config *config) { return max_pid_guess; }

size_t get_elf_interpreter_count_guess(const struct config *config) {
  return elf_interpreter_count_guess;
}

size_t get_queue_size_guess(const struct config *config) {
  return queue_size_guess;
}

const char *get_event_open_exec_not_editor(const struct config *config) {
  return event_open_exec_not_editor;
}

const char *get_event_open_exec_editor(const struct config *config) {
  return event_open_exec_editor;
}

const char *get_event_close_write_not_by_editor(const struct config *config) {
  return event_close_write_not_by_editor;
}

const char *get_event_close_write_by_editor(const struct config *config) {
  return event_close_write_by_editor;
}

const char *get_event_queue_head_deleted(const struct config *config) {
  return event_queue_head_deleted;
}

const char *get_event_queue_head_forbidden(const struct config *config) {
  return event_queue_head_forbidden;
}

const char *get_event_queue_head_stored(const struct config *config) {
  return event_queue_head_stored;
}

void free_config(struct config *config) {
  if (config) {
    free_set(config->editors);
    free_set(config->history_paths);
    free_set(config->excluded_paths);
    free_set(config->included_paths);
    free(config);
  }
}
