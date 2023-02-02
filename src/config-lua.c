#include "circbreak.h"
#include "config.h"
#include <errno.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdlib.h>
#include <string.h>

extern const char _binary_lua_config_lua_start;
extern const char _binary_lua_config_lua_end;
extern const char _binary_lua_validation_lua_start;
extern const char _binary_lua_validation_lua_end;

struct config {
  struct set *editors;
  char *store_root;
  char *queue_path;
  char *version_pattern;
  size_t debounce_seconds;
  size_t path_length_guess;
  size_t elf_interpreter_count_guess;
  size_t executable_count_guess;
  size_t queue_size_guess;
  pid_t max_pid_guess;
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
  return TNULL(strdup(lua_tostring(lua, -1)), trace);
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
    if (luaL_loadbuffer(lua, &_binary_lua_config_lua_start,
                        &_binary_lua_config_lua_end -
                            &_binary_lua_config_lua_start,
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
          luaL_loadbuffer(lua, &_binary_lua_validation_lua_start,
                          &_binary_lua_validation_lua_end -
                              &_binary_lua_validation_lua_start,
                          "validation") ||
          lua_pcall(lua, 0, 0, 0)) {
        throw_dynamic(lua_tostring(lua, -1), trace);
      }
      disarm(circuit_breaker);
    } else if (luaL_loadbuffer(lua, &_binary_lua_validation_lua_start,
                               &_binary_lua_validation_lua_end -
                                   &_binary_lua_validation_lua_start,
                               "validation") ||
               lua_pcall(lua, 0, 0, 0)) {
      throw_dynamic(lua_tostring(lua, -1), trace);
    }
  }

  free_circuit_breaker(circuit_breaker);

  config->editors = read_lua_set(lua, "editors", trace);
  config->store_root = read_lua_string(lua, "store_root", trace);
  config->queue_path = read_lua_string(lua, "queue_path", trace);
  config->version_pattern = read_lua_string(lua, "version_pattern", trace);

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

const struct set *get_configured_editors(const struct config *config) {
  return config->editors;
}

const char *get_configured_store_root(const struct config *config) {
  return config->store_root;
}

const char *get_configured_queue_path(const struct config *config) {
  return config->queue_path;
}

const char *get_configured_version_pattern(const struct config *config) {
  return config->version_pattern;
}

size_t get_configured_debounce_seconds(const struct config *config) {
  return config->debounce_seconds;
}

size_t get_configured_path_length_guess(const struct config *config) {
  return config->path_length_guess;
}

pid_t get_configured_max_pid_guess(const struct config *config) {
  return config->max_pid_guess;
}

size_t get_configured_elf_interpreter_count_guess(const struct config *config) {
  return config->elf_interpreter_count_guess;
}

size_t get_configured_executable_count_guess(const struct config *config) {
  return config->executable_count_guess;
}

size_t get_configured_queue_size_guess(const struct config *config) {
  return config->queue_size_guess;
}

void free_config(struct config *config) {
  if (config) {
    free_set(config->editors);
    free(config->store_root);
    free(config->queue_path);
    free(config->version_pattern);
    free(config);
  }
}
