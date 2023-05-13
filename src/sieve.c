#include "sieve.h"
#include "buffer.h"
#include "set.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>

struct sieved_path {
  size_t set_count;
  const char **ends;
  const char *hiding_dot;
};

struct sieved_path *sieve(const char *path, size_t relative_path_offset,
                          const struct set **sets, size_t set_count,
                          struct trace *trace) {
  assert(*path == '/');
  assert(relative_path_offset);

  struct sieved_path *sieved_path =
      TNULL(malloc(sizeof(struct sieved_path)), trace);
  const char **ends = TNULL(calloc(set_count, sizeof(char *)), trace);
  struct buffer *absolute_path_buffer = create_buffer(trace);
  struct buffer *relative_path_buffer = create_buffer(trace);

  if (!ok(trace)) {
    free(sieved_path);
    free(ends);
    free_buffer(absolute_path_buffer);
    free_buffer(relative_path_buffer);
    return NULL;
  }

  sieved_path->set_count = set_count;
  sieved_path->ends = ends;
  sieved_path->hiding_dot = NULL;

  const struct buffer_view *absolute_path_view = get_view(absolute_path_buffer);
  const struct buffer_view *relative_path_view = get_view(relative_path_buffer);

  for (const char *this = path, *next = path + 1; *this; ++this, ++next) {
    concat_char(*this, absolute_path_buffer, trace);
    if (this >= path + relative_path_offset) {
      concat_char(*this, relative_path_buffer, trace);
    }
    if (!ok(trace)) {
      free_buffer(absolute_path_buffer);
      free_buffer(relative_path_buffer);
      free_sieved_path(sieved_path);
      return NULL;
    }
    if (!*next || *next == '/' || this == path) {
      for (size_t i = 0; i < set_count; ++i) {
        if (is_within(absolute_path_view, sets[i]) ||
            (this >= path + relative_path_offset &&
             is_within(relative_path_view, sets[i]))) {
          sieved_path->ends[i] = next;
        }
      }
    }
    if (*this == '/' && *next == '.') {
      sieved_path->hiding_dot = next;
    }
  }

  free_buffer(absolute_path_buffer);
  free_buffer(relative_path_buffer);

  return sieved_path;
}

const char *get_hiding_dot(const struct sieved_path *sieved_path) {
  return sieved_path->hiding_dot;
};

const char *const *get_sieved_ends(const struct sieved_path *sieved_path) {
  return sieved_path->ends;
}

void free_sieved_path(struct sieved_path *sieved_path) {
  if (sieved_path) {
    free(sieved_path->ends);
    free(sieved_path);
  }
}
