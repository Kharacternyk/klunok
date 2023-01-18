#include "store.h"
#include "messages.h"
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

struct store *create_store(const char *root, struct trace *trace) {
  struct store *store = malloc(sizeof(struct store));
  if (!store) {
    trace_errno(trace);
    return NULL;
  }

  store->root = strdup(root);
  if (!store->root) {
    trace_errno(trace);
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
                   const struct store *store, struct trace *trace) {
  char *store_path = get_store_path(filesystem_path, version, store);
  if (!store_path) {
    return trace_errno(trace);
  }

  create_parents(store_path, S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH,
                 trace);
  if (!ok(trace)) {
    remove_empty_parents(store_path, trace);
    return free(store_path);
  }

  int in_fd = open(filesystem_path, O_RDONLY);
  if (in_fd < 0) {
    if (errno == ENOENT) {
      trace_static(messages.store.copy.file_does_not_exist, trace);
    } else if (errno == EACCES) {
      trace_static(messages.store.copy.permission_denied, trace);
    } else {
      trace_errno(trace);
    }
    remove_empty_parents(store_path, trace);
    return free(store_path);
  }

  int out_fd = open(store_path, O_CREAT | O_WRONLY | O_TRUNC,
                    S_IRUSR | S_IRGRP | S_IROTH);
  if (out_fd < 0) {
    trace_errno(trace);
    remove_empty_parents(store_path, trace);
    close(in_fd);
    return free(store_path);
  }

  struct stat in_fd_stat;
  if (fstat(in_fd, &in_fd_stat) < 0) {
    trace_errno(trace);
    remove_empty_parents(store_path, trace);
    close(out_fd);
    close(in_fd);
    return free(store_path);
  }

  while (in_fd_stat.st_size) {
    size_t written_size = sendfile(out_fd, in_fd, NULL, in_fd_stat.st_size);
    if (written_size > 0) {
      in_fd_stat.st_size -= written_size;
    } else {
      trace_errno(trace);
      remove_empty_parents(store_path, trace);
      close(out_fd);
      close(in_fd);
      return free(store_path);
    }
  }

  if (close(out_fd) < 0) {
    trace_errno(trace);
    remove_empty_parents(store_path, trace);
    close(in_fd);
    return free(store_path);
  }

  close(in_fd);
  free(store_path);
}

void free_store(struct store *store) {
  if (store) {
    free(store->root);
    free(store);
  }
}
