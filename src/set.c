#include "set.h"
#include "buffer.h"
#include "messages.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct entry {
  struct buffer *value;
  size_t count;
  struct entry *next;
};

struct set {
  size_t size;
  size_t empty_head_count;
  struct entry **heads;
};

struct set *create_set(size_t size_guess, struct trace *trace) {
  size_t size = size_guess * 2 + 2;
  struct entry **entries = TNULL(calloc(size, sizeof(struct entry *)), trace);
  struct set *set = TNULL(malloc(sizeof(struct set)), trace);
  if (!ok(trace)) {
    free(entries);
    return NULL;
  }
  set->size = size;
  set->empty_head_count = size;
  set->heads = entries;
  return set;
}

bool is_empty(const struct set *set) {
  assert(set->empty_head_count <= set->size);
  return set->empty_head_count == set->size;
}

static struct entry **get_head(const struct buffer_view *value,
                               const struct set *set) {
  return set->heads + get_hash(value) % set->size;
}

static struct entry **find_entry(const struct buffer_view *value,
                                 struct entry **head) {
  struct entry **entry = head;

  while (*entry) {
    assert((*entry)->count > 0);
    const struct buffer_view *candidate = get_view((*entry)->value);

    if (get_hash(value) == get_hash(candidate) &&
        get_length(value) == get_length(candidate) &&
        !strcmp(get_string(value), get_string(candidate))) {
      return entry;
    }

    entry = &((*entry)->next);
  }

  return entry;
}

size_t get_count(const struct buffer_view *value, const struct set *set) {
  if (is_empty(set)) {
    return 0;
  }
  struct entry **entry = find_entry(value, get_head(value, set));
  if (*entry) {
    return (*entry)->count;
  }
  return 0;
}

bool is_within(const struct buffer_view *value, const struct set *set) {
  return get_count(value, set);
}

void add(const char *value, struct set *set, struct trace *trace) {
  struct buffer *buffer = create_buffer(trace);
  concat_string(value, buffer, trace);
  if (!ok(trace)) {
    free_buffer(buffer);
    return;
  }

  struct entry **head = get_head(get_view(buffer), set);
  struct entry **entry = find_entry(get_view(buffer), head);
  if (*entry) {
    ++(*entry)->count;
    free_buffer(buffer);
    return;
  }

  struct entry *new_entry = TNULL(malloc(sizeof(struct entry)), trace);
  if (!ok(trace)) {
    free_buffer(buffer);
    free(new_entry);
    return;
  }

  if (!*head) {
    --set->empty_head_count;
  }

  new_entry->next = *head;
  new_entry->value = buffer;
  new_entry->count = 1;

  *head = new_entry;
}

void pop(const struct buffer_view *value, struct set *set) {
  struct entry **head = get_head(value, set);
  struct entry **entry = find_entry(value, head);
  if (!*entry) {
    return;
  }

  --(*entry)->count;

  if (!(*entry)->count) {
    struct entry *new_next = (*entry)->next;
    free_buffer((*entry)->value);
    free(*entry);
    *entry = new_next;

    if (!*head) {
      ++set->empty_head_count;
    }
  }
}

void free_set(struct set *set) {
  if (!set) {
    return;
  }
  for (size_t i = 0; i < set->size; ++i) {
    struct entry *entry = set->heads[i];
    while (entry) {
      struct entry *next = entry->next;
      free_buffer(entry->value);
      free(entry);
      entry = next;
    }
  }
  free(set->heads);
  free(set);
}
