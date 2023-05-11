#include "parents.h"
#include "messages.h"
#include "trace.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void create_parents(const char *original_path, struct trace *trace) {
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
    if (mkdir(path, 0755) < 0 && errno != EEXIST) {
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

static bool is_separator(char c) { return !c || c == '/'; }

size_t get_common_parent_path_length(const char *first, const char *second) {
  assert(*first == '/');
  assert(*first == *second);

  for (size_t i = 1, result = 1; /*keep going*/; ++i) {
    if (is_separator(first[i]) && is_separator(second[i])) {
      result = i + 1;
      if (!first[i] || !second[i]) {
        return result;
      }
    } else if (first[i] != second[i]) {
      return result;
    }
  }
}
