#include "linq.h"
#include "builder.h"
#include "messages.h"
#include "parents.h"
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

struct linq {
  int dirfd;
  time_t debounce_seconds;
  size_t head_index;
  size_t size;
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
  mode_t mode = S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH;
  create_parents(path, mode, trace);
  TNEG(mkdir(path, mode), trace);
}

static void free_entries(struct dirent **entries, size_t entry_count) {
  for (size_t i = 0; i < entry_count; ++i) {
    free(entries[i]);
  }
  free(entries);
}

static struct linq *load_or_create_linq(const char *path,
                                        time_t debounce_seconds,
                                        bool try_to_create,
                                        struct trace *trace) {
  if (!ok(trace)) {
    return NULL;
  }
  struct dirent **entries;
  int entry_count = scandir(path, &entries, dot_filter, compare);
  if (entry_count < 0) {
    if (errno == ENOENT && try_to_create) {
      create_linq_path(path, trace);
      return load_or_create_linq(path, debounce_seconds, false, trace);
    }
    throw_errno(trace);
    return NULL;
  }

  struct linq *linq = TNULL(malloc(sizeof(struct linq)), trace);
  int dirfd = TNEG(open(path, O_DIRECTORY), trace);
  if (!ok(trace)) {
    free(linq);
    free_entries(entries, entry_count);
    return NULL;
  }

  if (entry_count > 0) {
    linq->head_index = strtol(entries[0]->d_name, NULL, 10);
  } else {
    linq->head_index = 0;
  }

  linq->dirfd = dirfd;
  linq->size = entry_count;
  linq->debounce_seconds = debounce_seconds;

  free_entries(entries, entry_count);

  return linq;
}

struct linq *load_linq(const char *path, time_t debounce_seconds,
                       struct trace *trace) {
  return load_or_create_linq(path, debounce_seconds, true, trace);
}

void push_to_linq(const char *path, struct linq *linq, struct trace *trace) {
  struct builder *link_builder = create_builder(trace);
  concat_size(linq->head_index + linq->size, link_builder, trace);
  TNEG(symlinkat(path, linq->dirfd, build_string(link_builder)), trace);
  if (!ok(trace)) {
    return free_builder(link_builder);
  }

  ++linq->size;
  free_builder(link_builder);
}

char *pop_from_linq(struct linq *linq, size_t length_guess,
                    time_t *retry_after_seconds, struct trace *trace) {
  if (!ok(trace)) {
    return NULL;
  }
  if (!linq->size) {
    *retry_after_seconds = -1;
    return NULL;
  }

  struct builder *link_builder = create_builder(trace);
  concat_size(linq->head_index, link_builder, trace);
  struct stat link_stat;
  TNEG(fstatat(linq->dirfd, build_string(link_builder), &link_stat,
               AT_SYMLINK_NOFOLLOW),
       trace);
  if (!ok(trace)) {
    free_builder(link_builder);
    return NULL;
  }

  time_t link_age = time(NULL) - link_stat.st_mtime;
  if (link_age < linq->debounce_seconds) {
    *retry_after_seconds = linq->debounce_seconds - link_age;
    free_builder(link_builder);
    return NULL;
  }

  struct stat target_stat;
  if (fstatat(linq->dirfd, build_string(link_builder), &target_stat, 0) < 0 ||
      target_stat.st_mtime > link_stat.st_mtime) {
    TNEG(unlinkat(linq->dirfd, build_string(link_builder), 0), trace);
    if (!ok(trace)) {
      free_builder(link_builder);
      return NULL;
    }
    ++linq->head_index;
    --linq->size;
    free_builder(link_builder);
    return pop_from_linq(linq, length_guess, retry_after_seconds, trace);
  }

  size_t max_size = length_guess + 1;
  for (;;) {
    char *target = TNULL(malloc(max_size), trace);
    int length = TNEG(
        readlinkat(linq->dirfd, build_string(link_builder), target, max_size),
        trace);

    if (!ok(trace)) {
      free_builder(link_builder);
      free(target);
      return NULL;
    }
    if (length < max_size) {
      TNEG(unlinkat(linq->dirfd, build_string(link_builder), 0), trace);
      if (!ok(trace)) {
        free_builder(link_builder);
        free(target);
        return NULL;
      }
      ++linq->head_index;
      --linq->size;
      free_builder(link_builder);
      target[length] = 0;
      return target;
    }

    free(target);
    max_size *= 2;
  }
}

void free_linq(struct linq *linq) {
  if (linq) {
    close(linq->dirfd);
    free(linq);
  }
}
