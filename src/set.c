#include "set.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct entry {
  const char *value;
  struct entry *next;
};

struct set {
  size_t size;
  struct entry **entries;
};

struct set *create_set(size_t size_guess) {
  size_t size = size_guess * 2 + 2;
  struct entry **entries = calloc(size, sizeof(struct entry *));
  struct set *set = malloc(sizeof(struct set));

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

bool is_in_set(const char *value, struct set *set) {
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

void add_to_set(const char *value, struct set *set) {
  size_t hashed_value = hash(value);
  struct entry **entry = &(set->entries[hashed_value % set->size]);

  if (!*entry) {
    *entry = malloc(sizeof(struct entry));
    (*entry)->next = NULL;
    (*entry)->value = strdup(value);
    return;
  }

  while ((*entry)->next) {
    entry = &((*entry)->next);
  }

  (*entry)->next = malloc(sizeof(struct entry));
  (*entry)->next->next = NULL;
  (*entry)->next->value = strdup(value);
}
