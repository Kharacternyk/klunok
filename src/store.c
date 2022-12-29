#include "store.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

#define STORE_DIR_MODE (S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH)
#define STORE_FILE_MODE (S_IRUSR | S_IRGRP | S_IROTH)

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

static char *get_store_path(const char *filesystem_path, const char *version,
                            struct store *store,
                            struct callback *error_callback) {
  assert(*filesystem_path == '/');
  size_t length = store->root_length + strlen(filesystem_path) +
                  (/*a slash*/ 1) + strlen(version);
  char *store_path = malloc(length + (/*0 at the end*/ 1));
  if (!store_path) {
    invoke_callback(error_callback);
    return NULL;
  }
  sprintf(store_path, "%s%s/%s", store->root, filesystem_path, version);
  return store_path;
}

bool is_in_store(const char *filesystem_path, const char *version,
                 struct store *store, struct callback *error_callback) {
  char *store_path =
      get_store_path(filesystem_path, version, store, error_callback);
  if (!store_path) {
    return false;
  }
  bool result = access(store_path, F_OK) == 0;
  free(store_path);
  return result;
}

static void create_dirs(char **path, struct callback *error_callback) {
  char *slash = strchr(*path, '/');

  if (slash == *path) {
    slash = strchr(slash + 1, '/');
  }

  while (slash) {
    *slash = 0;
    if (mkdir(*path, STORE_DIR_MODE) < 0) {
      if (errno != EEXIST) {
        invoke_callback(error_callback);
        free(*path);
        *path = NULL;
        return;
      }
    }
    *slash = '/';
    ++slash;
    slash = strchr(slash, '/');
  }
}

void link_to_store(const char *filesystem_path, const char *version,
                   struct store *store, struct callback *error_callback) {
  char *store_path =
      get_store_path(filesystem_path, version, store, error_callback);
  if (!store_path) {
    return;
  }
  create_dirs(&store_path, error_callback);
  if (!store_path) {
    return;
  }

  if (link(filesystem_path, store_path) < 0) {
    invoke_callback(error_callback);
  }

  free(store_path);
}

void copy_to_store(const char *filesystem_path, const char *version,
                   struct store *store, struct callback *error_callback) {
  char *store_path =
      get_store_path(filesystem_path, version, store, error_callback);
  if (!store_path) {
    return;
  }
  create_dirs(&store_path, error_callback);
  if (!store_path) {
    return;
  }

  int in_fd = open(filesystem_path, O_RDONLY);

  if (in_fd > 0) {
    int out_fd =
        open(store_path, O_CREAT | O_WRONLY | O_TRUNC, STORE_FILE_MODE);

    if (out_fd > 0) {
      struct stat in_fd_stat;

      if (fstat(in_fd, &in_fd_stat) >= 0) {
        while (in_fd_stat.st_size) {
          size_t written_size =
              sendfile(out_fd, in_fd, NULL, in_fd_stat.st_size);
          if (written_size > 0) {
            in_fd_stat.st_size -= written_size;
          } else {
            invoke_callback(error_callback);
            break;
          }
        }
      } else {
        invoke_callback(error_callback);
      }
    } else {
      invoke_callback(error_callback);
    }
  } else {
    invoke_callback(error_callback);
  }

  free(store_path);
}
