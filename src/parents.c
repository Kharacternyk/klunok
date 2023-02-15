#include "parents.h"
#include "messages.h"
#include "trace.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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
      throw_errno(trace);
      throw_context(path, trace);
      throw_static(messages.parents.cannot_create_ancestor, trace);
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
        throw_errno(trace);
        throw_context(path, trace);
        throw_static(messages.parents.cannot_remove_ancestor, trace);
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
