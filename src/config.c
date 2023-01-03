#include "config.h"
#include <lauxlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

struct config {
  struct set *editors;
  char *version_pattern;
};

struct config *load_config(const char *path,
                           const struct callback *error_callback,
                           const char **error_message) {
  struct config *config = malloc(sizeof(struct config));
  if (!config) {
    invoke_callback(error_callback);
    return NULL;
  }
  config->editors = NULL;
  config->version_pattern = NULL;

  bool is_error = false;
  struct callback *wrapped_error_callback =
      create_simple_callback(&is_error, error_callback);
  if (!wrapped_error_callback) {
    invoke_callback(error_callback);
    free_config(config);
    return NULL;
  }

  lua_State *lua = luaL_newstate();
  if (luaL_loadfile(lua, path) || lua_pcall(lua, 0, 0, 0)) {
    *error_message = lua_tostring(lua, -1);
    invoke_callback(error_callback);
    goto cleanup;
  }

  lua_getglobal(lua, "version_pattern");
  if (!lua_isstring(lua, -1)) {
    *error_message = "`version_pattern` must be a string";
    invoke_callback(error_callback);
    goto cleanup;
  }
  config->version_pattern = strdup(lua_tostring(lua, -1));

  lua_getglobal(lua, "editors");
  if (!lua_istable(lua, -1)) {
    *error_message = "`editors` must be a list of strings";
    invoke_callback(error_callback);
    goto cleanup;
  }

  config->editors = create_set(lua_rawlen(lua, -1), wrapped_error_callback);
  if (is_error) {
    invoke_callback(error_callback);
    goto cleanup;
  }

  lua_pushnil(lua);
  while (lua_next(lua, -2)) {
    if (!lua_isstring(lua, -1)) {
      *error_message = "`editors` must be a list of strings";
      invoke_callback(error_callback);
      goto cleanup;
    }

    char *editor = strdup(lua_tostring(lua, -1));
    add_to_set(editor, config->editors, wrapped_error_callback);
    if (is_error) {
      invoke_callback(error_callback);
      free(editor);
      goto cleanup;
    }

    lua_pop(lua, 1);
  }

  lua_close(lua);
  free(wrapped_error_callback);
  return config;

cleanup:
  lua_close(lua);
  free(wrapped_error_callback);
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
