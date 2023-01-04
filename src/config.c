#include "config.h"
#include <errno.h>
#include <lauxlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

struct config {
  struct set *editors;
  char *version_pattern;
};

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
  if (luaL_loadfile(lua, path) || lua_pcall(lua, 0, 0, 0)) {
    *dynamic_error_message = strdup(lua_tostring(lua, -1));
    if (!*dynamic_error_message) {
      *static_error_message = "Lua syntax error";
    }
    goto fail;
  }

  lua_getglobal(lua, "version_pattern");
  if (!lua_isstring(lua, -1)) {
    *static_error_message = "`version_pattern` must be a string";
    goto fail;
  }
  config->version_pattern = strdup(lua_tostring(lua, -1));

  lua_getglobal(lua, "editors");
  if (!lua_istable(lua, -1)) {
    *static_error_message = "`editors` must be a list of strings";
    goto fail;
  }

  config->editors = create_set(lua_rawlen(lua, -1), error_code);
  if (*error_code) {
    goto fail;
  }

  lua_pushnil(lua);
  while (lua_next(lua, -2)) {
    if (!lua_isstring(lua, -1)) {
      *static_error_message = "`editors` must be a list of strings";
      goto fail;
    }

    add_to_set(lua_tostring(lua, -1), config->editors, error_code);
    if (*error_code) {
      goto fail;
    }

    lua_pop(lua, 1);
  }

  lua_close(lua);
  return config;

fail:
  lua_close(lua);
  free_config(config);
  return NULL;
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
