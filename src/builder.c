#include "builder.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct builder {
  char *buffer;
  size_t size;
};

struct builder *create_builder(struct trace *trace) {
  if (!ok(trace)) {
    return NULL;
  }
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

void concat_string(const char *string, struct builder *builder,
                   struct trace *trace) {
  if (!ok(trace)) {
    return;
  }
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

void concat_size(size_t size, struct builder *builder, struct trace *trace) {
  if (!ok(trace)) {
    return;
  }
  if (!size) {
    return concat_string("0", builder, trace);
  }

  size_t power_of_ten = 1;
  while (power_of_ten <= size / 10) {
    power_of_ten *= 10;
  }

  char digit[] = {0, 0};
  while (size) {
    digit[0] = '0' + size / power_of_ten;
    concat_string(digit, builder, trace);
    size %= power_of_ten;
    power_of_ten /= 10;
  }
}

const char *build_string(const struct builder *builder) {
  return builder->buffer;
}

size_t get_builder_length(const struct builder *builder) {
  return builder->size - 1;
}

void truncate_builder(size_t length, struct builder *builder) {
  assert(length < builder->size);
  builder->buffer[length] = 0;
  builder->size = length + 1;
}

void free_builder(struct builder *builder) {
  if (builder) {
    free(builder->buffer);
    free(builder);
  }
}
