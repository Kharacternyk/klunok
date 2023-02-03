#include "parents.h"
#include "unistd.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void create_parents(const char *original_path, mode_t mode,
                    struct trace *trace) {
  char *path = TNULL(strdup(original_path), trace);
  if (!ok(trace)) {
    return;
  }

  char *slash = strchr(path, '/');

  if (slash == path) {
    slash = strchr(slash + 1, '/');
  }

  while (slash) {
    *slash = 0;
    if (mkdir(path, mode) < 0 && errno != EEXIST) {
      throw_context(path, trace);
      throw_errno(trace);
      free(path);
      return;
    }
    *slash = '/';
    ++slash;
    slash = strchr(slash, '/');
  }

  free(path);
}

void remove_empty_parents(const char *original_path, struct trace *trace) {
  char *path = TNULL(strdup(original_path), trace);
  if (!ok(trace)) {
    return;
  }

  char *slash = strrchr(path, '/');

  while (slash && slash != path) {
    *slash = 0;
    if (rmdir(path) < 0) {
      if (errno != ENOTEMPTY) {
        throw_context(path, trace);
        throw_errno(trace);
      }
      free(path);
      return;
    }
    char *old_slash = slash;
    slash = strrchr(path, '/');
    *old_slash = '/';
  }

  free(path);
}
