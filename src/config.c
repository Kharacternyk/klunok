#include "config.h"
#include <lauxlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static const size_t editor_count_guess = 100;

struct config {
  struct set *editors;
};

struct config *load_config(const char *path, struct callback *error_callback,
                           const char **error_message) {
  struct config *config = malloc(sizeof(struct config));
  if (!config) {
    invoke_callback(error_callback);
    return NULL;
  }

  bool is_error = false;
  struct callback *wrapped_error_callback =
      create_simple_callback(&is_error, error_callback);
  if (!wrapped_error_callback) {
    invoke_callback(error_callback);
    free(config);
    return NULL;
  }

  config->editors = create_set(editor_count_guess, wrapped_error_callback);
  if (is_error) {
    invoke_callback(error_callback);
    free(config);
    free(wrapped_error_callback);
    return NULL;
  }

  lua_State *lua = luaL_newstate();
  if (luaL_loadfile(lua, path) || lua_pcall(lua, 0, 0, 0)) {
    *error_message = lua_tostring(lua, -1);
    invoke_callback(error_callback);
    goto cleanup;
  }

  lua_getglobal(lua, "editors");
  if (!lua_istable(lua, -1)) {
    *error_message = "`editors` must be a list of strings";
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

  free(wrapped_error_callback);
  lua_close(lua);
  return config;

cleanup:
  free(wrapped_error_callback);
  lua_close(lua);
  free_config(config);
  return NULL;
}

struct set *get_editors_from_config(struct config *config) {
  return config->editors;
}

void free_config(struct config *config) {
  free_set(config->editors);
  free(config);
}
