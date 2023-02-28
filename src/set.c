#include "set.h"
#include "messages.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct entry {
  char *value;
  size_t count;
  size_t length;
  struct entry *next;
};

struct set {
  size_t size;
  struct entry **entries;
};

struct hashed_value {
  const char *value;
  size_t hash;
  size_t length;
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

static size_t next_hash(size_t hash, char character) {
  hash += character;
  hash += hash << 10;
  hash ^= hash >> 6;
  return hash;
}

static size_t final_hash(size_t hash) {
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}

static struct hashed_value get_hashed_value(const char *value) {
  struct hashed_value hashed_value = {
      .value = value,
  };
  while (*value) {
    hashed_value.hash = next_hash(hashed_value.hash, *value);
    ++hashed_value.length;
    ++value;
  }
  hashed_value.hash = final_hash(hashed_value.hash);
  return hashed_value;
}

static struct entry **get_head(size_t hash, const struct set *set) {
  return &(set->entries[hash % set->size]);
}

static struct entry **get_entry(struct hashed_value hashed_value,
                                const struct set *set) {
  struct entry **entry = get_head(hashed_value.hash, set);

  while (*entry) {
    assert((*entry)->count > 0);
    bool is_equal = false;
    if ((*entry)->length == hashed_value.length) {
      is_equal =
          !memcmp(hashed_value.value, (*entry)->value, hashed_value.length);
    }
    if (is_equal) {
      return entry;
    }
    entry = &((*entry)->next);
  }

  return entry;
}

static struct entry *create_entry(size_t count,
                                  struct hashed_value hashed_value,
                                  struct entry *next, struct trace *trace) {
  assert(count);
  char *value_copy = TNULL(strdup(hashed_value.value), trace);
  struct entry *entry = TNULL(malloc(sizeof(struct entry)), trace);
  if (!ok(trace)) {
    free(value_copy);
    return NULL;
  }

  entry->next = next;
  entry->value = value_copy;
  entry->count = count;
  entry->length = hashed_value.length;

  return entry;
}

static void remove_entry(struct entry **entry) {
  struct entry *new_next = (*entry)->next;
  free((*entry)->value);
  free(*entry);
  *entry = new_next;
}

size_t get_count_in_set(const char *value, const struct set *set) {
  struct entry **entry = get_entry(get_hashed_value(value), set);
  if (*entry) {
    return (*entry)->count;
  }
  return 0;
}

void set_count_in_set(size_t count, const char *value, struct set *set,
                      struct trace *trace) {
  if (!ok(trace)) {
    return;
  }

  struct hashed_value hashed_value = get_hashed_value(value);
  struct entry **entry = get_entry(hashed_value, set);
  if (*entry) {
    if (!count) {
      remove_entry(entry);
    } else {
      (*entry)->count = count;
    }
    return;
  }

  struct entry **head = get_head(hashed_value.hash, set);
  struct entry *new_entry = create_entry(count, hashed_value, *head, trace);
  if (ok(trace)) {
    *head = new_entry;
  }
}

bool is_in_set(const char *value, const struct set *set) {
  return get_count_in_set(value, set);
}

void add_to_set(const char *value, struct set *set, struct trace *trace) {
  if (!ok(trace)) {
    return;
  }

  struct hashed_value hashed_value = get_hashed_value(value);
  struct entry **entry = get_entry(hashed_value, set);
  if (*entry) {
    ++(*entry)->count;
    return;
  }

  struct entry **head = get_head(hashed_value.hash, set);
  struct entry *new_entry = create_entry(1, hashed_value, *head, trace);
  if (ok(trace)) {
    *head = new_entry;
  }
}

void remove_from_set(const char *value, struct set *set) {
  struct entry **entry = get_entry(get_hashed_value(value), set);
  if (!*entry) {
    return;
  }

  --(*entry)->count;

  if (!(*entry)->count) {
    remove_entry(entry);
  }
}

size_t get_best_match_count_in_set(const char *value, char separator,
                                   const struct set *set) {
  size_t best_match_count = 0;
  struct hashed_value hashed_value = {
      .value = value,
  };
  while (*value) {
    do {
      hashed_value.hash = next_hash(hashed_value.hash, *value);
      ++hashed_value.length;
      ++value;
    } while (*value && *value != separator);

    size_t hash = hashed_value.hash;
    hashed_value.hash = final_hash(hash);
    struct entry **entry = get_entry(hashed_value, set);
    hashed_value.hash = hash;

    if (*entry) {
      best_match_count = (*entry)->count;
    }
  }
  return best_match_count;
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
