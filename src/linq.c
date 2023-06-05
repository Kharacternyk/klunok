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

struct linq_head {
  char *target;
  char *path;
  size_t metadata;
  time_t pause;
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

static char *strip_metadata(char *target) {
  while (target[1] == '/' || (target[1] == '.' && target[2] == '/')) {
    ++target;
  }
  return target;
}

static char *read_entry(const char *entry, const struct linq *linq,
                        struct trace *trace) {
  if (!ok(trace)) {
    return NULL;
  }
  size_t max_size = linq->length_guess + 1;
  for (;;) {
    try(trace);
    char *target = TNULL(malloc(max_size), trace);
    int length = TNEG(readlinkat(linq->dirfd, entry, target, max_size), trace);
    rethrow_context(entry, trace);
    finally_rethrow_static(messages.linq.invalid_entry, trace);

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
    if (ok(trace)) {
      add(strip_metadata(entry_target), set, trace);
      free(entry_target);
    }
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

static void concat_metadata(struct buffer *buffer, size_t metadata,
                            struct trace *trace) {
  size_t bit_length = 0;

  while (metadata >= 1 << bit_length) {
    ++bit_length;
  }

  for (size_t i = 1; ok(trace) && i <= bit_length; ++i) {
    concat_char('/', buffer, trace);
    if (metadata & (1 << (bit_length - i))) {
      concat_char('.', buffer, trace);
    }
  }
}

void push(const char *path, size_t metadata, struct linq *linq,
          struct trace *trace) {
  if (!ok(trace)) {
    return;
  }
  assert(*path == '/');

  struct buffer *link_buffer = create_buffer(trace);
  concat_size(linq->head_index + linq->size, link_buffer, trace);

  struct buffer *target_buffer = create_buffer(trace);
  concat_metadata(target_buffer, metadata, trace);
  concat_string(path, target_buffer, trace);

  TNEG(symlinkat(get_string(get_view(target_buffer)), linq->dirfd,
                 get_string(get_view(link_buffer))),
       trace);
  add(path, linq->set, trace);
  if (ok(trace)) {
    ++linq->size;
  }
  free_buffer(link_buffer);
  free_buffer(target_buffer);
}

struct linq_head *get_head(struct linq *linq, struct trace *trace) {
  struct linq_head *head = TNULL(malloc(sizeof(struct linq_head)), trace);
  if (!ok(trace)) {
    return NULL;
  }

  if (!linq->size) {
    head->pause = -1;
    return head;
  }

  struct buffer *link = create_buffer(trace);
  concat_size(linq->head_index, link, trace);

  struct stat link_stat;
  TNEG(fstatat(linq->dirfd, get_string(get_view(link)), &link_stat,
               AT_SYMLINK_NOFOLLOW),
       trace);
  if (!ok(trace)) {
    free_buffer(link);
    free(head);
    return NULL;
  }

  time_t link_age = time(NULL) - link_stat.st_mtime;
  if (link_age < linq->debounce_seconds) {
    free_buffer(link);
    head->pause = linq->debounce_seconds - link_age;
    return head;
  }

  char *target = read_entry(get_string(get_view(link)), linq, trace);
  free_buffer(link);

  char *path = target;
  head->metadata = 0;
  while (path[1] == '/' || (path[1] == '.' && path[2] == '/')) {
    head->metadata *= 2;
    ++path;
    if (*path == '.') {
      head->metadata += 1;
      ++path;
    }
  }

  struct buffer_view *path_view = create_buffer_view(path, trace);

  if (ok(trace) && get_count(path_view, linq->set) > 1) {
    free(target);
    free(head);
    free_buffer_view(path_view);
    pop_head(linq, trace);
    return get_head(linq, trace);
  }

  free_buffer_view(path_view);

  if (!ok(trace)) {
    free(head);
    return NULL;
  }

  head->pause = 0;
  head->target = target;
  head->path = path;

  return head;
}

void pop_head(struct linq *linq, struct trace *trace) {
  if (!ok(trace)) {
    return;
  }
  assert(linq->size);
  struct buffer *link = create_buffer(trace);
  concat_size(linq->head_index, link, trace);
  char *target = read_entry(get_string(get_view(link)), linq, trace);

  TNEG(unlinkat(linq->dirfd, get_string(get_view(link)), 0), trace);

  if (ok(trace)) {
    struct buffer_view *path_view =
        create_buffer_view(strip_metadata(target), trace);

    if (ok(trace)) {
      pop(path_view, linq->set);
      --linq->size;
      if (linq->size) {
        ++linq->head_index;
      } else {
        linq->head_index = 0;
      }
    }

    free_buffer_view(path_view);
  }

  free(target);
  free_buffer(link);
}

const char *get_path(const struct linq_head *head) { return head->path; }

size_t get_metadata(const struct linq_head *head) { return head->metadata; }

time_t get_pause(const struct linq_head *head) { return head->pause; }

void redebounce(time_t debounce_seconds, struct linq *linq) {
  linq->debounce_seconds = debounce_seconds;
}

void free_linq(struct linq *linq) {
  if (linq) {
    close(linq->dirfd);
    free_set(linq->set);
    free(linq);
  }
}

void free_linq_head(struct linq_head *head) {
  if (head) {
    if (!head->pause) {
      free(head->target);
    }
    free(head);
  }
}
