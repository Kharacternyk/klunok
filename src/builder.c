#include "builder.h"
#include <stdlib.h>
#include <string.h>

struct builder {
  char *buffer;
  size_t size;
};

struct builder *create_builder(struct trace *trace) {
  struct builder *builder = malloc(sizeof(struct builder));
  if (!builder) {
    throw_errno(trace);
    return NULL;
  }
  builder->size = 1;
  builder->buffer = calloc(1, sizeof(char));
  if (!builder->buffer) {
    throw_errno(trace);
    free(builder);
    return NULL;
  }
  return builder;
}

void append_to_builder(const char *string, struct builder *builder,
                       struct trace *trace) {
  size_t string_length = strlen(string);
  if (string_length > 0) {
    char *new_buffer = realloc(builder->buffer, builder->size + string_length);
    if (!new_buffer) {
      return throw_errno(trace);
    }
    strcat(new_buffer, string);
    builder->buffer = new_buffer;
    builder->size += string_length;
  }
}

const char *build_string(const struct builder *builder) {
  return builder->buffer;
}

void free_builder(struct builder *builder) {
  if (builder) {
    free(builder->buffer);
    free(builder);
  }
}
