#include "set.h"
#include "messages.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct entry {
  char *value;
  size_t count;
  struct entry *next;
};

struct set {
  size_t size;
  struct entry **entries;
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
  set->entries = entries;
  return set;
}

static size_t hash(const char *value) {
  size_t result = 0;

  while (*value) {
    result += *value;
    result += result << 10;
    result ^= result >> 6;
    ++value;
  }

  result += result << 3;
  result ^= result >> 11;
  result += result << 15;

  return result;
}

static struct entry **get_head(const char *value, const struct set *set) {
  size_t hashed_value = hash(value);
  return &(set->entries[hashed_value % set->size]);
}

static struct entry **get_entry(const char *value, const struct set *set) {
  struct entry **entry = get_head(value, set);

  while (*entry) {
    assert((*entry)->count > 0);
    if (strcmp(value, (*entry)->value) == 0) {
      return entry;
    }
    entry = &((*entry)->next);
  }

  return entry;
}

bool is_in_set(const char *value, const struct set *set) {
  return *get_entry(value, set);
}

bool is_unique_within_set(const char *value, const struct set *set) {
  struct entry **entry = get_entry(value, set);
  return !*entry || (*entry)->count == 1;
}

void add_to_set(const char *value, struct set *set, struct trace *trace) {
  if (!ok(trace)) {
    return;
  }

  struct entry **entry = get_entry(value, set);
  if (*entry) {
    ++(*entry)->count;
    return;
  }

  char *value_copy = TNULL(strdup(value), trace);
  struct entry *new_entry = TNULL(malloc(sizeof(struct entry)), trace);
  if (!ok(trace)) {
    return free(value_copy);
  }

  struct entry **head = get_head(value, set);

  new_entry->next = *head;
  new_entry->value = value_copy;
  new_entry->count = 1;

  *head = new_entry;
}

void remove_from_set(const char *value, struct set *set, struct trace *trace) {
  if (!ok(trace)) {
    return;
  }

  struct entry **entry = get_entry(value, set);
  if (!*entry) {
    return throw_static(messages.set.not_in_set, trace);
  }

  --(*entry)->count;

  if (!(*entry)->count) {
    struct entry *new_next = (*entry)->next;
    free((*entry)->value);
    free(*entry);
    *entry = new_next;
  }
}

void free_set(struct set *set) {
  if (!set) {
    return;
  }
  for (size_t i = 0; i < set->size; ++i) {
    struct entry *entry = set->entries[i];
    while (entry) {
      struct entry *next = entry->next;
      free(entry->value);
      free(entry);
      entry = next;
    }
  }
  free(set->entries);
  free(set);
}
