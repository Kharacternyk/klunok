#include "storepath.h"
#include "buffer.h"
#include "extension.h"
#include "timestamp.h"
#include "trace.h"
#include <stdlib.h>
#include <string.h>

struct store_path {
  struct buffer *buffer;
  char *extension;
  size_t extensionless_length;
  size_t duplicate_count;
};

struct store_path *create_store_path(const char *root,
                                     const char *relative_path,
                                     const char *version, struct trace *trace) {
  struct store_path *store_path =
      TNULL(malloc(sizeof(struct store_path)), trace);
  struct buffer *buffer = create_buffer(trace);
  concat_string(root, buffer, trace);
  concat_char('/', buffer, trace);
  concat_string(relative_path, buffer, trace);
  concat_char('/', buffer, trace);
  concat_string(version, buffer, trace);

  if (!ok(trace)) {
    free_buffer(buffer);
    free(store_path);
    return NULL;
  }

  store_path->buffer = buffer;
  store_path->duplicate_count = 0;
  store_path->extensionless_length = get_length(get_view(buffer));
  store_path->extension =
      TNULL(strdup(get_file_extension(relative_path)), trace);
  concat_string(store_path->extension, buffer, trace);

  if (!ok(trace)) {
    free_store_path(store_path);
    return NULL;
  }

  return store_path;
}

const char *get_current_path(const struct store_path *store_path) {
  return get_string(get_view(store_path->buffer));
}

void increment(struct store_path *store_path, struct trace *trace) {
  if (!ok(trace)) {
    return;
  }

  set_length(store_path->extensionless_length, store_path->buffer);
  /*FIXME configure me*/
  concat_char('-', store_path->buffer, trace);
  concat_size(store_path->duplicate_count + 1, store_path->buffer, trace);
  concat_string(store_path->extension, store_path->buffer, trace);

  if (ok(trace)) {
    ++store_path->duplicate_count;
  }
}

void free_store_path(struct store_path *store_path) {
  if (store_path) {
    free_buffer(store_path->buffer);
    free(store_path->extension);
    free(store_path);
  }
}
