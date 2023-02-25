#include "circbreak.h"
#include "config.h"
#include "set.h"
#include "trace.h"
#include <errno.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdlib.h>
#include <string.h>

extern const char _binary_lua_pre_config_lua_start;
extern const char _binary_lua_pre_config_lua_end;
extern const char _binary_lua_post_config_lua_start;
extern const char _binary_lua_post_config_lua_end;

struct config {
  struct set *editors;
  struct set *history_paths;
  char *store_root;
  char *queue_path;
  char *journal_path;
  char *journal_timestamp_pattern;
  char *version_pattern;
  size_t debounce_seconds;
  size_t path_length_guess;
  size_t elf_interpreter_count_guess;
  size_t executable_count_guess;
  size_t queue_size_guess;
  pid_t max_pid_guess;
  char *event_open_exec_not_editor;
  char *event_open_exec_editor;
  char *event_open_exec_interpreter;
  char *event_close_write_not_by_editor;
  char *event_close_write_by_editor;
  char *event_queue_head_deleted;
  char *event_queue_head_forbidden;
  char *event_queue_head_stored;
};

static size_t read_lua_size(lua_State *lua, const char *name) {
  lua_getglobal(lua, name);
  return lua_tointeger(lua, -1);
}

static char *read_lua_string(lua_State *lua, const char *name,
                             struct trace *trace) {
  if (!ok(trace)) {
    return NULL;
  }
  lua_getglobal(lua, name);
  const char *string = lua_tostring(lua, -1);
  if (!string) {
    return NULL;
  }
  return TNULL(strdup(string), trace);
}

static struct set *read_lua_set(lua_State *lua, const char *name,
                                struct trace *trace) {
  if (!ok(trace)) {
    return NULL;
  }
  lua_getglobal(lua, name);

  struct set *set = create_set(lua_rawlen(lua, -1), trace);
  if (!ok(trace)) {
    return NULL;
  }

  lua_pushnil(lua);
  while (lua_next(lua, -2)) {
    add_to_set(lua_tostring(lua, -2), set, trace);
    if (!ok(trace)) {
      free_set(set);
      return NULL;
    }

    lua_pop(lua, 1);
  }

  return set;
}

struct config *load_config(const char *path, struct trace *trace) {
  struct config *config = TNULL(calloc(1, sizeof(struct config)), trace);
  if (!ok(trace)) {
    return NULL;
  }

  lua_State *lua = TNULL(luaL_newstate(), trace);

  if (ok(trace)) {
    luaL_openlibs(lua);
    if (luaL_loadbuffer(lua, &_binary_lua_pre_config_lua_start,
                        &_binary_lua_pre_config_lua_end -
                            &_binary_lua_pre_config_lua_start,
                        "default") ||
        lua_pcall(lua, 0, 0, 0)) {
      throw_dynamic(lua_tostring(lua, -1), trace);
    }
  }

  struct circuit_breaker *circuit_breaker = create_circuit_breaker(
      read_lua_size(lua, "circuit_breaker_seconds"), trace);

  if (ok(trace)) {
    if (path) {
      arm(circuit_breaker);
      if (luaL_loadfile(lua, path) || lua_pcall(lua, 0, 0, 0) ||
          luaL_loadbuffer(lua, &_binary_lua_post_config_lua_start,
                          &_binary_lua_post_config_lua_end -
                              &_binary_lua_post_config_lua_start,
                          "validation") ||
          lua_pcall(lua, 0, 0, 0)) {
        throw_dynamic(lua_tostring(lua, -1), trace);
      }
      disarm(circuit_breaker);
    } else if (luaL_loadbuffer(lua, &_binary_lua_post_config_lua_start,
                               &_binary_lua_post_config_lua_end -
                                   &_binary_lua_post_config_lua_start,
                               "validation") ||
               lua_pcall(lua, 0, 0, 0)) {
      throw_dynamic(lua_tostring(lua, -1), trace);
    }
  }

  free_circuit_breaker(circuit_breaker);

  config->editors = read_lua_set(lua, "editors", trace);
  config->history_paths = read_lua_set(lua, "history_paths", trace);
  config->store_root = read_lua_string(lua, "store_root", trace);
  config->queue_path = read_lua_string(lua, "queue_path", trace);
  config->journal_path = read_lua_string(lua, "journal_path", trace);
  config->journal_timestamp_pattern =
      read_lua_string(lua, "journal_timestamp_pattern", trace);
  config->version_pattern = read_lua_string(lua, "version_pattern", trace);
  config->event_open_exec_not_editor =
      read_lua_string(lua, "event_open_exec_not_editor", trace);
  config->event_open_exec_editor =
      read_lua_string(lua, "event_open_exec_editor", trace);
  config->event_open_exec_interpreter =
      read_lua_string(lua, "event_open_exec_interpreter", trace);
  config->event_close_write_not_by_editor =
      read_lua_string(lua, "event_close_write_not_by_editor", trace);
  config->event_close_write_by_editor =
      read_lua_string(lua, "event_close_write_by_editor", trace);
  config->event_queue_head_deleted =
      read_lua_string(lua, "event_queue_head_deleted", trace);
  config->event_queue_head_forbidden =
      read_lua_string(lua, "event_queue_head_forbidden", trace);
  config->event_queue_head_stored =
      read_lua_string(lua, "event_queue_head_stored", trace);

  if (!ok(trace)) {
    if (lua) {
      lua_close(lua);
    }
    free_config(config);
    return NULL;
  }

  config->debounce_seconds = read_lua_size(lua, "debounce_seconds");
  config->path_length_guess = read_lua_size(lua, "path_length_guess");
  config->max_pid_guess = read_lua_size(lua, "max_pid_guess");
  config->elf_interpreter_count_guess =
      read_lua_size(lua, "elf_interpreter_count_guess");
  config->executable_count_guess = read_lua_size(lua, "executable_count_guess");
  config->queue_size_guess = read_lua_size(lua, "queue_size_guess");

  lua_close(lua);
  return config;
}

const struct set *get_editors(const struct config *config) {
  return config->editors;
}

const struct set *get_history_paths(const struct config *config) {
  return config->history_paths;
}

const char *get_store_root(const struct config *config) {
  return config->store_root;
}

const char *get_queue_path(const struct config *config) {
  return config->queue_path;
}

const char *get_journal_path(const struct config *config) {
  return config->journal_path;
}

const char *get_journal_timestamp_pattern(const struct config *config) {
  return config->journal_timestamp_pattern;
}

const char *get_version_pattern(const struct config *config) {
  return config->version_pattern;
}

size_t get_debounce_seconds(const struct config *config) {
  return config->debounce_seconds;
}

size_t get_path_length_guess(const struct config *config) {
  return config->path_length_guess;
}

pid_t get_max_pid_guess(const struct config *config) {
  return config->max_pid_guess;
}

size_t get_elf_interpreter_count_guess(const struct config *config) {
  return config->elf_interpreter_count_guess;
}

size_t get_executable_count_guess(const struct config *config) {
  return config->executable_count_guess;
}

size_t get_queue_size_guess(const struct config *config) {
  return config->queue_size_guess;
}

const char *get_event_open_exec_not_editor(const struct config *config) {
  return config->event_open_exec_not_editor;
}

const char *get_event_open_exec_editor(const struct config *config) {
  return config->event_open_exec_editor;
}

const char *get_event_open_exec_interpreter(const struct config *config) {
  return config->event_open_exec_interpreter;
}

const char *get_event_close_write_not_by_editor(const struct config *config) {
  return config->event_close_write_not_by_editor;
}

const char *get_event_close_write_by_editor(const struct config *config) {
  return config->event_close_write_by_editor;
}

const char *get_event_queue_head_deleted(const struct config *config) {
  return config->event_queue_head_deleted;
}

const char *get_event_queue_head_forbidden(const struct config *config) {
  return config->event_queue_head_forbidden;
}

const char *get_event_queue_head_stored(const struct config *config) {
  return config->event_queue_head_stored;
}

void free_config(struct config *config) {
  if (config) {
    free_set(config->editors);
    free_set(config->history_paths);
    free(config->store_root);
    free(config->queue_path);
    free(config->journal_path);
    free(config->journal_timestamp_pattern);
    free(config->version_pattern);
    free(config->event_open_exec_not_editor);
    free(config->event_open_exec_editor);
    free(config->event_open_exec_interpreter);
    free(config->event_close_write_not_by_editor);
    free(config->event_close_write_by_editor);
    free(config->event_queue_head_deleted);
    free(config->event_queue_head_forbidden);
    free(config->event_queue_head_stored);
    free(config);
  }
}
