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

struct store {
  const char *root;
  size_t root_length;
};

struct store *create_store(const char *root,
                           const struct callback *error_callback) {
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
                            const struct store *store) {
  assert(*filesystem_path == '/');
  size_t length = store->root_length + strlen(filesystem_path) +
                  (/*a slash*/ 1) + strlen(version);
  char *store_path = malloc(length + (/*0 at the end*/ 1));
  if (!store_path) {
    return NULL;
  }
  sprintf(store_path, "%s%s/%s", store->root, filesystem_path, version);
  return store_path;
}

static bool create_dirs(char *path) {
  char *slash = strchr(path, '/');

  if (slash == path) {
    slash = strchr(slash + 1, '/');
  }

  while (slash) {
    *slash = 0;
    if (mkdir(path, S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH) < 0 &&
        errno != EEXIST) {
      *slash = '/';
      return false;
    }
    *slash = '/';
    ++slash;
    slash = strchr(slash, '/');
  }

  return true;
}

static bool remove_dirs(char *path) {
  char *slash = strrchr(path, '/');

  while (slash && slash != path) {
    *slash = 0;
    if (rmdir(path) < 0) {
      *slash = '/';
      return errno == ENOTEMPTY;
    }
    char *old_slash = slash;
    slash = strrchr(path, '/');
    *old_slash = '/';
  }

  return true;
}

static void rollback(char *path, const struct callback *error_callback) {
  invoke_callback(error_callback);
  if (!remove_dirs(path)) {
    invoke_callback(error_callback);
  }
}

void copy_to_store(const char *filesystem_path, const char *version,
                   const struct store *store,
                   const struct callback *error_callback) {
  char *store_path = get_store_path(filesystem_path, version, store);
  if (!store_path) {
    invoke_callback(error_callback);
    return;
  }
  if (!create_dirs(store_path)) {
    rollback(store_path, error_callback);
    goto path_cleanup;
  }

  int in_fd = open(filesystem_path, O_RDONLY);
  if (in_fd < 0) {
    rollback(store_path, error_callback);
    goto path_cleanup;
  }

  int out_fd = open(store_path, O_CREAT | O_WRONLY | O_TRUNC,
                    S_IRUSR | S_IRGRP | S_IROTH);
  if (out_fd < 0) {
    rollback(store_path, error_callback);
    goto in_fd_cleanup;
  }

  struct stat in_fd_stat;
  if (fstat(in_fd, &in_fd_stat) < 0) {
    rollback(store_path, error_callback);
    goto out_fd_cleanup;
  }

  while (in_fd_stat.st_size) {
    size_t written_size = sendfile(out_fd, in_fd, NULL, in_fd_stat.st_size);
    if (written_size > 0) {
      in_fd_stat.st_size -= written_size;
    } else {
      rollback(store_path, error_callback);
      goto out_fd_cleanup;
    }
  }

out_fd_cleanup:
  close(out_fd);
in_fd_cleanup:
  close(in_fd);
path_cleanup:
  free(store_path);
}
