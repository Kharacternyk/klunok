#include "linq.h"
#include "buffer.h"
#include "messages.h"
#include "parents.h"
#include "set.h"
#include "trace.h"
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

struct linq {
  int dirfd;
  time_t debounce_seconds;
  size_t length_guess;
  size_t head_index;
  size_t size;
  struct set *set;
};

static int dot_filter(const struct dirent *dirent) {
  const char *name = dirent->d_name;
  return !(name[0] == '.' &&
           (name[1] == 0 || (name[1] == '.' && name[2] == 0)));
}

static int compare(const struct dirent **first, const struct dirent **second) {
  return strtol((*first)->d_name, NULL, 10) -
         strtol((*second)->d_name, NULL, 10);
}

static void create_linq_path(const char *path, struct trace *trace) {
  create_parents(path, trace);
  TNEG(mkdir(path, 0755), trace);
}

static void free_entries(struct dirent **entries, size_t entry_count) {
  for (size_t i = 0; i < entry_count; ++i) {
    free(entries[i]);
  }
  free(entries);
}

static char *read_entry(const char *entry, const struct linq *linq,
                        struct trace *trace) {
  size_t max_size = linq->length_guess + 1;
  for (;;) {
    rethrow_check(trace);
    char *target = TNULL(malloc(max_size), trace);
    int length = TNEG(readlinkat(linq->dirfd, entry, target, max_size), trace);
    rethrow_context(entry, trace);
    rethrow_static(messages.linq.invalid_entry, trace);

    if (!ok(trace)) {
      free(target);
      return NULL;
    }
    if (length < max_size) {
      target[length] = 0;
      return target;
    }

    free(target);
    max_size *= 2;
  }
}

static struct linq *
load_or_create_linq(const char *path, time_t debounce_seconds,
                    size_t entry_count_guess, size_t entry_length_guess,
                    bool try_to_create, struct trace *trace) {
  if (!ok(trace)) {
    return NULL;
  }
  struct dirent **entries;
  int entry_count = scandir(path, &entries, dot_filter, compare);
  if (entry_count < 0) {
    if (errno == ENOENT && try_to_create) {
      create_linq_path(path, trace);
      return load_or_create_linq(path, debounce_seconds, entry_count_guess,
                                 entry_length_guess, false, trace);
    }
    throw_errno(trace);
    return NULL;
  }

  size_t set_size =
      entry_count_guess > entry_count ? entry_count_guess : entry_count;

  struct linq *linq = TNULL(malloc(sizeof(struct linq)), trace);
  int dirfd = TNEG(open(path, O_DIRECTORY), trace);
  struct set *set = create_set(set_size, trace);
  if (ok(trace)) {
    linq->dirfd = dirfd;
    linq->size = entry_count;
    linq->debounce_seconds = debounce_seconds;
    linq->length_guess = entry_length_guess;
    linq->set = set;
    if (entry_count > 0) {
      linq->head_index = strtol(entries[0]->d_name, NULL, 10);
    } else {
      linq->head_index = 0;
    }
  }
  for (size_t i = 0; i < entry_count && ok(trace); ++i) {
    char *entry_target = read_entry(entries[i]->d_name, linq, trace);
    add_to_set(entry_target, set, trace);
    free(entry_target);
  }
  if (!ok(trace)) {
    free(linq);
    free_entries(entries, entry_count);
    free_set(set);
    return NULL;
  }

  free_entries(entries, entry_count);

  return linq;
}

struct linq *load_linq(const char *path, time_t debounce_seconds,
                       size_t entry_count_guess, size_t entry_length_guess,
                       struct trace *trace) {
  return load_or_create_linq(path, debounce_seconds, entry_count_guess,
                             entry_length_guess, true, trace);
}

void push_to_linq(const char *path, struct linq *linq, struct trace *trace) {
  struct buffer *link_buffer = create_buffer(trace);
  concat_size(linq->head_index + linq->size, link_buffer, trace);
  TNEG(symlinkat(path, linq->dirfd, get_string(link_buffer)), trace);
  add_to_set(path, linq->set, trace);
  if (ok(trace)) {
    ++linq->size;
  }
  free_buffer(link_buffer);
}

char *get_linq_head(struct linq *linq, time_t *retry_after_seconds,
                    struct trace *trace) {
  if (!ok(trace)) {
    return NULL;
  }
  if (!linq->size) {
    *retry_after_seconds = -1;
    return NULL;
  }

  struct buffer *link_buffer = create_buffer(trace);
  concat_size(linq->head_index, link_buffer, trace);
  struct stat link_stat;
  TNEG(fstatat(linq->dirfd, get_string(link_buffer), &link_stat,
               AT_SYMLINK_NOFOLLOW),
       trace);
  if (!ok(trace)) {
    free_buffer(link_buffer);
    return NULL;
  }

  time_t link_age = time(NULL) - link_stat.st_mtime;
  if (link_age < linq->debounce_seconds) {
    *retry_after_seconds = linq->debounce_seconds - link_age;
    free_buffer(link_buffer);
    return NULL;
  }

  char *target = read_entry(get_string(link_buffer), linq, trace);
  free_buffer(link_buffer);
  if (ok(trace) && get_count_in_set(target, linq->set) > 1) {
    pop_from_linq(linq, trace);
    return get_linq_head(linq, retry_after_seconds, trace);
  }

  *retry_after_seconds = 0;
  return target;
}

void pop_from_linq(struct linq *linq, struct trace *trace) {
  assert(linq->size);
  struct buffer *link_buffer = create_buffer(trace);
  concat_size(linq->head_index, link_buffer, trace);
  char *target = read_entry(get_string(link_buffer), linq, trace);
  TNEG(unlinkat(linq->dirfd, get_string(link_buffer), 0), trace);
  if (ok(trace)) {
    remove_from_set(target, linq->set);
    --linq->size;
    if (linq->size) {
      ++linq->head_index;
    } else {
      linq->head_index = 0;
    }
  }
  free(target);
  free_buffer(link_buffer);
}

void redebounce_linq(time_t debounce_seconds, struct linq *linq) {
  linq->debounce_seconds = debounce_seconds;
}

void free_linq(struct linq *linq) {
  if (linq) {
    close(linq->dirfd);
    free_set(linq->set);
    free(linq);
  }
}
