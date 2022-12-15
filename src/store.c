#include "store.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

struct store {
  const char *path;
  size_t path_length;
};

struct store *create_store(const char *path) {
  struct store *store = malloc(sizeof(struct store));
  store->path = path;
  store->path_length = strlen(path);
  return store;
}

bool is_in_store(const char *path, struct store *store) {
  size_t length = store->path_length + (/*a slash*/ 1) + strlen(path);
  char *buffer = malloc(length);

  sprintf(buffer, "%s/%s", store->path, path);

  bool result = access(buffer, F_OK) == 0;
  free(buffer);

  return result;
}

void link_to_store(const char *path, struct store *store) { /*TODO*/
}
