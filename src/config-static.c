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
static const size_t debounce_seconds = 60;
static const size_t path_length_guess = 1024;
static const pid_t max_pid_guess = 1 << 15;
static const size_t elf_interpreter_count_guess = 1;
static const size_t executable_count_guess = 128;
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

static const char *const event_open_exec_not_editor;
static const char *const event_open_exec_editor;
static const char *const event_open_exec_interpreter;
static const char *const event_close_write_not_by_editor;
static const char *const event_close_write_by_editor;
static const char *const event_queue_head_deleted;
static const char *const event_queue_head_forbidden;
static const char *const event_queue_head_stored = "";

struct config {
  struct set *editors;
  struct set *history_paths;
};

struct config *load_config(const char *path, struct trace *trace) {
  if (ok(trace) && path) {
    throw_static(messages.config.is_static, trace);
    return NULL;
  }

  struct config *config = TNULL(calloc(1, sizeof(struct config)), trace);
  if (!ok(trace)) {
    return NULL;
  }

  size_t editors_length = sizeof editors / sizeof editors[0];
  config->editors = create_set(editors_length, trace);
  for (size_t i = 0; ok(trace) && i < editors_length; ++i) {
    add_to_set(editors[i], config->editors, trace);
  }

  config->history_paths = create_set(0, trace);

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

size_t get_executable_count_guess(const struct config *config) {
  return executable_count_guess;
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

const char *get_event_open_exec_interpreter(const struct config *config) {
  return event_open_exec_interpreter;
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
    free(config);
  }
}
