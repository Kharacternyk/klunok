#include "store.h"
#include "parents.h"
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
  char *root;
  size_t root_length;
};

struct store *create_store(const char *root, int *error_code) {
  struct store *store = malloc(sizeof(struct store));
  if (!store) {
    *error_code = errno;
    return NULL;
  }

  store->root = strdup(root);
  if (!store->root) {
    *error_code = errno;
    free(store);
    return NULL;
  }

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

void copy_to_store(const char *filesystem_path, const char *version,
                   const struct store *store, int *error_code,
                   int *cleanup_error_code) {
  char *store_path = get_store_path(filesystem_path, version, store);
  if (!store_path) {
    *error_code = errno;
    return;
  }

  create_parents(store_path, S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH,
                 error_code);
  if (*error_code) {
    remove_empty_parents(store_path, cleanup_error_code);
    goto path_cleanup;
  }

  int in_fd = open(filesystem_path, O_RDONLY);
  if (in_fd < 0) {
    *error_code = errno;
    remove_empty_parents(store_path, cleanup_error_code);
    goto path_cleanup;
  }

  int out_fd = open(store_path, O_CREAT | O_WRONLY | O_TRUNC,
                    S_IRUSR | S_IRGRP | S_IROTH);
  if (out_fd < 0) {
    *error_code = errno;
    remove_empty_parents(store_path, cleanup_error_code);
    goto in_fd_cleanup;
  }

  struct stat in_fd_stat;
  if (fstat(in_fd, &in_fd_stat) < 0) {
    *error_code = errno;
    remove_empty_parents(store_path, cleanup_error_code);
    goto out_fd_cleanup;
  }

  while (in_fd_stat.st_size) {
    size_t written_size = sendfile(out_fd, in_fd, NULL, in_fd_stat.st_size);
    if (written_size > 0) {
      in_fd_stat.st_size -= written_size;
    } else {
      *error_code = errno;
      remove_empty_parents(store_path, cleanup_error_code);
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

void free_store(struct store *store) {
  if (store) {
    free(store->root);
    free(store);
  }
}
