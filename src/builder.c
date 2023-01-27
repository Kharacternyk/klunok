#include "builder.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct builder {
  char *buffer;
  size_t size;
};

struct builder *create_builder(struct trace *trace) {
  struct builder *builder = TNULL(malloc(sizeof(struct builder)), trace);
  char *buffer = TNULL(calloc(1, sizeof(char)), trace);
  if (!ok(trace)) {
    free(builder);
    free(buffer);
    return NULL;
  }
  builder->size = 1;
  builder->buffer = buffer;
  return builder;
}

void concat_string(const char *string, struct builder *builder,
                   struct trace *trace) {
  if (!ok(trace)) {
    return;
  }
  size_t string_length = strlen(string);
  if (string_length > 0) {
    char *new_buffer =
        TNULL(realloc(builder->buffer, builder->size + string_length), trace);
    if (!ok(trace)) {
      return;
    }
    strcat(new_buffer, string);
    builder->buffer = new_buffer;
    builder->size += string_length;
  }
}

void concat_char(char c, struct builder *builder, struct trace *trace) {
  char *new_buffer = TNULL(realloc(builder->buffer, builder->size + 1), trace);
  if (!ok(trace)) {
    return;
  }
  builder->buffer = new_buffer;
  builder->buffer[builder->size] = '\0';
  builder->buffer[builder->size - 1] = c;
  ++builder->size;
}

void concat_size(size_t size, struct builder *builder, struct trace *trace) {
  if (!size) {
    return concat_char('0', builder, trace);
  }
  if (!ok(trace)) {
    return;
  }
  size_t saved_length = get_builder_length(builder);

  size_t power_of_ten = 1;
  while (power_of_ten <= size / 10) {
    power_of_ten *= 10;
  }

  char digit;
  while (power_of_ten) {
    digit = '0' + size / power_of_ten;
    concat_char(digit, builder, trace);
    if (!ok(trace)) {
      truncate_builder(saved_length, builder);
      return;
    }
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
