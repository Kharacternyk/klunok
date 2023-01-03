#include "set.h"
#include <stdlib.h>
#include <string.h>

struct entry {
  char *value;
  struct entry *next;
};

struct set {
  size_t size;
  struct entry **entries;
};

struct set *create_set(size_t size_guess,
                       const struct callback *error_callback) {
  size_t size = size_guess * 2 + 2;
  struct entry **entries = calloc(size, sizeof(struct entry *));
  if (!entries) {
    invoke_callback(error_callback);
    return NULL;
  }

  struct set *set = malloc(sizeof(struct set));
  if (!set) {
    invoke_callback(error_callback);
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

bool is_in_set(const char *value, const struct set *set) {
  size_t hashed_value = hash(value);
  struct entry *entry = set->entries[hashed_value % set->size];

  while (entry) {
    if (strcmp(value, entry->value) == 0) {
      return true;
    }
    entry = entry->next;
  }

  return false;
}

void add_to_set(const char *value, struct set *set,
                const struct callback *error_callback) {
  size_t hashed_value = hash(value);
  struct entry **entry = &(set->entries[hashed_value % set->size]);

  char *value_copy = strdup(value);
  if (!value_copy) {
    invoke_callback(error_callback);
    return;
  }

  struct entry *new_entry = malloc(sizeof(struct entry));
  if (!new_entry) {
    invoke_callback(error_callback);
    free(value_copy);
    return;
  }
  new_entry->next = NULL;
  new_entry->value = value_copy;

  if (!*entry) {
    *entry = new_entry;
    return;
  }

  while ((*entry)->next) {
    entry = &((*entry)->next);
  }

  (*entry)->next = new_entry;
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
