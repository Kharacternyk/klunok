#include "buffer.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct buffer {
  char *string;
  size_t size;
  struct hash *hash;
};

struct hash {
  size_t value;
  size_t seen_length;
};

static const struct hash empty_hash;

struct buffer *create_buffer(struct trace *trace) {
  struct buffer *buffer = TNULL(malloc(sizeof(struct buffer)), trace);
  struct hash *hash = TNULL(calloc(1, sizeof(struct hash)), trace);
  char *string = TNULL(calloc(1, sizeof(char)), trace);

  if (!ok(trace)) {
    free(buffer);
    free(string);
    return NULL;
  }

  buffer->string = string;
  buffer->size = 1;
  buffer->hash = hash;

  return buffer;
}

void concat_string(const char *string, struct buffer *buffer,
                   struct trace *trace) {
  if (!ok(trace)) {
    return;
  }
  size_t string_length = strlen(string);
  if (string_length > 0) {
    char *new_string =
        TNULL(realloc(buffer->string, buffer->size + string_length), trace);
    if (!ok(trace)) {
      return;
    }
    strcat(new_string, string);
    buffer->string = new_string;
    buffer->size += string_length;
  }
}

void concat_char(char c, struct buffer *buffer, struct trace *trace) {
  char *new_string = TNULL(realloc(buffer->string, buffer->size + 1), trace);
  if (!ok(trace)) {
    return;
  }
  buffer->string = new_string;
  buffer->string[buffer->size] = '\0';
  buffer->string[buffer->size - 1] = c;
  ++buffer->size;
}

void concat_size(size_t size, struct buffer *buffer, struct trace *trace) {
  if (!size) {
    return concat_char('0', buffer, trace);
  }
  if (!ok(trace)) {
    return;
  }
  size_t saved_length = get_length(buffer);

  size_t power_of_ten = 1;
  while (power_of_ten <= size / 10) {
    power_of_ten *= 10;
  }

  char digit;
  while (power_of_ten) {
    digit = '0' + size / power_of_ten;
    concat_char(digit, buffer, trace);
    if (!ok(trace)) {
      set_length(saved_length, buffer);
      return;
    }
    size %= power_of_ten;
    power_of_ten /= 10;
  }
}

const char *get_string(const struct buffer *buffer) { return buffer->string; }

size_t get_length(const struct buffer *buffer) { return buffer->size - 1; }

void set_length(size_t length, struct buffer *buffer) {
  assert(length < buffer->size);
  buffer->string[length] = 0;
  buffer->size = length + 1;
  *buffer->hash = empty_hash;
}

size_t get_hash(const struct buffer *buffer) {
  while (buffer->hash->seen_length < get_length(buffer)) {
    char character = buffer->string[buffer->hash->seen_length];
    size_t hash = buffer->hash->value;

    hash = character + (hash << 6) + (hash << 16) - hash;

    buffer->hash->value = hash;
    ++buffer->hash->seen_length;
  }

  return buffer->hash->value;
}

char *free_outer_buffer(struct buffer *buffer) {
  if (!buffer) {
    return NULL;
  }
  char *result = buffer->string;
  free(buffer->hash);
  free(buffer);
  return result;
}

void free_buffer(struct buffer *buffer) {
  if (buffer) {
    free(buffer->string);
    free(buffer->hash);
    free(buffer);
  }
}
