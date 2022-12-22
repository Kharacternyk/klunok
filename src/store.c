#include "store.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define STORE_DIR_MODE (S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH)

struct store {
  const char *root;
  size_t root_length;
};

struct store *create_store(const char *root, struct callback *error_callback) {
  struct store *store = malloc(sizeof(struct store));
  if (!store) {
    invoke_callback(error_callback);
    return NULL;
  }
  store->root = root;
  store->root_length = strlen(root);
  return store;
}

static char *get_store_path(const char *filesystem_path, struct store *store,
                            struct callback *error_callback) {
  assert(*filesystem_path == '/');
  size_t length =
      store->root_length + strlen(filesystem_path) + (/*0 at the end*/ 1);
  char *store_path = malloc(length);
  if (!store_path) {
    invoke_callback(error_callback);
    return NULL;
  }
  sprintf(store_path, "%s%s", store->root, filesystem_path);
  return store_path;
}

bool is_in_store(const char *filesystem_path, struct store *store,
                 struct callback *error_callback) {
  char *store_path = get_store_path(filesystem_path, store, error_callback);
  if (!store_path) {
    return false;
  }
  bool result = access(store_path, F_OK) == 0;
  free(store_path);
  return result;
}

void link_to_store(const char *filesystem_path, struct store *store,
                   struct callback *error_callback) {
  char *store_path = get_store_path(filesystem_path, store, error_callback);
  if (!store_path) {
    return;
  }
  char *slash = store_path + store->root_length;

  while (slash) {
    *slash = 0;
    if (mkdir(store_path, STORE_DIR_MODE) < 0) {
      if (errno != EEXIST) {
        invoke_callback(error_callback);
        free(store_path);
        return;
      }
    }
    *slash = '/';
    ++slash;
    slash = strchr(slash, '/');
  }

  if (link(filesystem_path, store_path) < 0) {
    invoke_callback(error_callback);
  }

  free(store_path);
}
