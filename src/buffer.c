#include "buffer.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct buffer_view {
  const char *string;
  size_t size;
  struct hash *hash;
};

struct buffer {
  char *string;
  size_t capacity;
  struct buffer_view view;
};

struct hash {
  size_t value;
  size_t seen_length;
};

static const struct hash empty_hash;

struct buffer_view *create_buffer_view(const char *string,
                                       struct trace *trace) {
  struct buffer_view *view = TNULL(malloc(sizeof(struct buffer_view)), trace);
  struct hash *hash = TNULL(calloc(1, sizeof(struct hash)), trace);

  if (!ok(trace)) {
    free(view);
    free(hash);
    return NULL;
  }

  view->string = string;
  view->hash = hash;
  view->size = strlen(string) + 1;

  return view;
}

struct buffer *create_buffer(struct trace *trace) {
  struct buffer *buffer = TNULL(malloc(sizeof(struct buffer)), trace);
  char *string = TNULL(calloc(1, sizeof(char)), trace);
  struct hash *hash = TNULL(calloc(1, sizeof(struct hash)), trace);

  if (!ok(trace)) {
    free(buffer);
    free(string);
    free(hash);
    return NULL;
  }

  buffer->string = string;
  buffer->capacity = 1;
  buffer->view.hash = hash;
  buffer->view.size = 1;
  buffer->view.string = buffer->string;

  return buffer;
}

static void ensure_capacity(size_t capacity, struct buffer *buffer,
                            struct trace *trace) {
  if (ok(trace) && capacity > buffer->capacity) {
    size_t new_capacity = capacity * 2;
    char *new_string = TNULL(realloc(buffer->string, new_capacity), trace);
    if (ok(trace)) {
      buffer->capacity = new_capacity;
      buffer->view.string = buffer->string = new_string;
    }
  }
}

void concat_string(const char *string, struct buffer *buffer,
                   struct trace *trace) {
  if (!ok(trace)) {
    return;
  }

  size_t string_length = strlen(string);
  size_t new_size = buffer->view.size + string_length;
  ensure_capacity(new_size, buffer, trace);

  if (ok(trace)) {
    strcat(buffer->string, string);
    buffer->view.size = new_size;
  }
}

void concat_char(char c, struct buffer *buffer, struct trace *trace) {
  ensure_capacity(buffer->view.size + 1, buffer, trace);
  if (ok(trace)) {
    buffer->string[buffer->view.size] = '\0';
    buffer->string[buffer->view.size - 1] = c;
    ++buffer->view.size;
  }
}

void concat_size(size_t size, struct buffer *buffer, struct trace *trace) {
  if (!ok(trace)) {
    return;
  }
  if (!size) {
    return concat_char('0', buffer, trace);
  }
  size_t saved_length = get_length(&buffer->view);

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

const struct buffer_view *get_view(const struct buffer *buffer) {
  return &buffer->view;
}

const char *get_string(const struct buffer_view *view) { return view->string; }

size_t get_length(const struct buffer_view *view) { return view->size - 1; }

void set_length(size_t length, struct buffer *buffer) {
  assert(length < buffer->view.size);
  buffer->string[length] = 0;
  buffer->view.size = length + 1;
  *buffer->view.hash = empty_hash;
}

size_t get_hash(const struct buffer_view *view) {
  while (view->hash->seen_length < get_length(view)) {
    char character = view->string[view->hash->seen_length];
    size_t hash = view->hash->value;

    hash = character + (hash << 6) + (hash << 16) - hash;

    view->hash->value = hash;
    ++view->hash->seen_length;
  }

  return view->hash->value;
}

void free_buffer_view(struct buffer_view *view) {
  if (view) {
    free(view->hash);
    free(view);
  }
}

char *free_outer_buffer(struct buffer *buffer) {
  if (!buffer) {
    return NULL;
  }
  char *result = buffer->string;
  free(buffer->view.hash);
  free(buffer);
  return result;
}

void free_buffer(struct buffer *buffer) {
  if (buffer) {
    free(buffer->string);
    free(buffer->view.hash);
    free(buffer);
  }
}
