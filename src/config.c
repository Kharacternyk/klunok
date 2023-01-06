#include "config.h"
#include <errno.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

extern const char _binary_lua_config_lua_start;
extern const char _binary_lua_config_lua_end;
extern const char _binary_lua_validation_lua_start;
extern const char _binary_lua_validation_lua_end;

struct config {
  struct set *editors;
  char *version_pattern;
  size_t version_max_length;
};

static size_t read_lua_size(lua_State *lua, const char *name) {
  lua_getglobal(lua, name);
  return lua_tointeger(lua, -1);
}

static char *read_lua_string(lua_State *lua, const char *name,
                             int *error_code) {
  lua_getglobal(lua, name);
  char *string = strdup(lua_tostring(lua, -1));
  if (!string) {
    *error_code = errno;
  }
  return string;
}

static struct set *read_lua_set(lua_State *lua, const char *name,
                                int *error_code) {
  lua_getglobal(lua, name);

  struct set *set = create_set(lua_rawlen(lua, -1), error_code);
  if (*error_code) {
    return NULL;
  }

  lua_pushnil(lua);
  while (lua_next(lua, -2)) {
    add_to_set(lua_tostring(lua, -2), set, error_code);
    if (*error_code) {
      free_set(set);
      return NULL;
    }

    lua_pop(lua, 1);
  }

  return set;
}

struct config *load_config(const char *path, int *error_code,
                           char **error_message) {
  struct config *config = malloc(sizeof(struct config));
  if (!config) {
    *error_code = errno;
    return NULL;
  }

  lua_State *lua = luaL_newstate();
  luaL_openlibs(lua);
  if (luaL_loadbuffer(lua, &_binary_lua_config_lua_start,
                      &_binary_lua_config_lua_end -
                          &_binary_lua_config_lua_start,
                      "default") ||
      lua_pcall(lua, 0, 0, 0) || luaL_loadfile(lua, path) ||
      lua_pcall(lua, 0, 0, 0) ||
      luaL_loadbuffer(lua, &_binary_lua_validation_lua_start,
                      &_binary_lua_validation_lua_end -
                          &_binary_lua_validation_lua_start,
                      "validation") ||
      lua_pcall(lua, 0, 0, 0)) {
    *error_message = strdup(lua_tostring(lua, -1));
    if (!*error_message) {
      *error_code = errno;
    }
    goto config_cleanup;
  }

  config->editors = read_lua_set(lua, "editors", error_code);
  if (*error_code) {
    goto config_cleanup;
  }

  config->version_pattern = read_lua_string(lua, "version_pattern", error_code);
  if (*error_code) {
    goto editors_cleanup;
  }

  config->version_max_length = read_lua_size(lua, "version_max_length");

  lua_close(lua);
  return config;

editors_cleanup:
  free_set(config->editors);
config_cleanup:
  free(config);
  lua_close(lua);
  return NULL;
}

const struct set *get_configured_editors(const struct config *config) {
  return config->editors;
}

const char *get_configured_version_pattern(const struct config *config) {
  return config->version_pattern;
}

const size_t get_configured_version_max_length(const struct config *config) {
  return config->version_max_length;
}

void free_config(struct config *config) {
  if (config) {
    free_set(config->editors);
    free(config->version_pattern);
    free(config);
  }
}
