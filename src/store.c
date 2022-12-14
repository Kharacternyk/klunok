#include "store.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

struct store {
  const char *path;
};

struct store *create_store(const char *path) {
  struct store *store = malloc(sizeof(struct store));
  store->path = path;
  return store;
}

bool is_in_store(const char *path, struct store *store) {
  char buffer[PATH_MAX];
  snprintf(buffer, PATH_MAX, "%s/%s", store->path, path);
  return access(buffer, F_OK) == 0;
}

void link_to_store(const char *path, struct store *store) { /*TODO*/
}
