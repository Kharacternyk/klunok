#include "config.h"
#include <errno.h>
#include <lauxlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

extern char _binary_lua_config_lua_start;
extern char _binary_lua_config_lua_end;

struct config {
  struct set *editors;
  char *version_pattern;
};

static char *read_lua_string(lua_State *lua, const char *name, int *error_code,
                             const char **static_error_message,
                             char **dynamic_error_message) {
  lua_getglobal(lua, name);
  if (!lua_isstring(lua, -1)) {
    *static_error_message = "Cannot read a string";
    const char *format = "`%s` must be a string";
    size_t message_length = snprintf(NULL, 0, format, name);
    *dynamic_error_message = malloc(message_length + 1);
    if (*dynamic_error_message) {
      snprintf(*dynamic_error_message, message_length + 1, format, name);
    }
    return NULL;
  }

  char *string = strdup(lua_tostring(lua, -1));
  if (!string) {
    *error_code = errno;
  }
  return string;
}

static struct set *read_lua_set(lua_State *lua, const char *name,
                                int *error_code,
                                const char **static_error_message,
                                char **dynamic_error_message) {
  lua_getglobal(lua, name);
  if (!lua_istable(lua, -1)) {
    goto lua_fail;
  }

  struct set *set = create_set(lua_rawlen(lua, -1), error_code);
  if (*error_code) {
    return NULL;
  }

  lua_pushnil(lua);
  while (lua_next(lua, -2)) {
    if (lua_type(lua, -2) != LUA_TSTRING) {
      free_set(set);
      goto lua_fail;
    }

    if (!lua_isnil(lua, -1)) {
      add_to_set(lua_tostring(lua, -2), set, error_code);
      if (*error_code) {
        free_set(set);
        return NULL;
      }
    }

    lua_pop(lua, 1);
  }

  return set;

  const char *format = "`%s` must be a list of strings";
  size_t message_length;

lua_fail:
  message_length = snprintf(NULL, 0, format, name);
  *static_error_message = "Cannot read a set";
  *dynamic_error_message = malloc(message_length + 1);
  if (*dynamic_error_message) {
    snprintf(*dynamic_error_message, message_length + 1, format, name);
  }
  return NULL;
}

struct config *load_config(const char *path, int *error_code,
                           const char **static_error_message,
                           char **dynamic_error_message) {
  struct config *config = malloc(sizeof(struct config));
  if (!config) {
    *error_code = errno;
    return NULL;
  }
  config->editors = NULL;
  config->version_pattern = NULL;

  lua_State *lua = luaL_newstate();
  if (luaL_loadbuffer(lua, &_binary_lua_config_lua_start,
                      &_binary_lua_config_lua_end -
                          &_binary_lua_config_lua_start,
                      "default") ||
      lua_pcall(lua, 0, 0, 0) || luaL_loadfile(lua, path) ||
      lua_pcall(lua, 0, 0, 0)) {
    *static_error_message = "Lua syntax error";
    *dynamic_error_message = strdup(lua_tostring(lua, -1));
    free(config);
    lua_close(lua);
    return NULL;
  }

  config->editors = read_lua_set(lua, "editors", error_code,
                                 static_error_message, dynamic_error_message);
  if (*error_code || *static_error_message) {
    free(config);
    lua_close(lua);
    return NULL;
  }

  config->version_pattern =
      read_lua_string(lua, "version_pattern", error_code, static_error_message,
                      dynamic_error_message);
  if (*error_code || *static_error_message) {
    free(config->editors);
    free(config);
    lua_close(lua);
    return NULL;
  }

  lua_close(lua);
  return config;
}

const struct set *get_configured_editors(const struct config *config) {
  return config->editors;
}

const char *get_configured_version_pattern(const struct config *config) {
  return config->version_pattern;
}

void free_config(struct config *config) {
  if (config) {
    free_set(config->editors);
    free(config->version_pattern);
    free(config);
  }
}
