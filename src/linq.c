#include "linq.h"
#include "parents.h"
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
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

static char *stringify(size_t value, int *error_code) {
  size_t length = snprintf(NULL, 0, "%zd", value);
  size_t size = length + 1;
  char *result = malloc(size);
  if (!result) {
    *error_code = errno;
    return NULL;
  }
  snprintf(result, size, "%zd", value);
  return result;
}

static void create_linq_path(const char *path, int *error_code) {
  mode_t mode = S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH;
  create_parents(path, mode, error_code);
  if (*error_code) {
    return;
  }
  if (mkdir(path, mode) < 0) {
    *error_code = errno;
  }
}

static void free_entries(struct dirent **entries, size_t entry_count) {
  for (size_t i = 0; i < entry_count; ++i) {
    free(entries[i]);
  }
  free(entries);
}

static struct linq *load_or_create_linq(const char *path,
                                        time_t debounce_seconds,
                                        bool try_to_create, int *error_code) {
  struct dirent **entries;
  int entry_count = scandir(path, &entries, dot_filter, compare);
  if (entry_count < 0) {
    if (errno == ENOENT && try_to_create) {
      create_linq_path(path, error_code);
      if (*error_code) {
        return NULL;
      }
      return load_or_create_linq(path, debounce_seconds, false, error_code);
    }
    *error_code = errno;
    return NULL;
  }

  struct linq *linq = malloc(sizeof(struct linq));
  if (!linq) {
    *error_code = errno;
    free_entries(entries, entry_count);
    return NULL;
  }

  if (entry_count > 0) {
    linq->head_index = strtol(entries[0]->d_name, NULL, 10);
  } else {
    linq->head_index = 0;
  }

  linq->size = entry_count;
  linq->debounce_seconds = debounce_seconds;

  linq->dirfd = open(path, O_DIRECTORY);
  if (linq->dirfd < 0) {
    *error_code = errno;
    free(linq);
    free_entries(entries, entry_count);
    return NULL;
  }

  return linq;
}

struct linq *load_linq(const char *path, time_t debounce_seconds,
                       int *error_code) {
  return load_or_create_linq(path, debounce_seconds, true, error_code);
}

void push_to_linq(const char *path, struct linq *linq, int *error_code) {
  char *link_name = stringify(linq->head_index + linq->size, error_code);
  if (*error_code) {
    return;
  }

  if (symlinkat(path, linq->dirfd, link_name) < 0) {
    *error_code = errno;
  } else {
    ++linq->size;
  }

  free(link_name);
}

char *pop_from_linq(struct linq *linq, size_t length_guess,
                    time_t *retry_after_seconds, int *error_code) {
  if (!linq->size) {
    *retry_after_seconds = -1;
    return NULL;
  }

  char *link_name = stringify(linq->head_index, error_code);
  if (*error_code) {
    return NULL;
  }

  struct stat link_stat;
  if (fstatat(linq->dirfd, link_name, &link_stat, AT_SYMLINK_NOFOLLOW) < 0) {
    *error_code = errno;
    free(link_name);
    return NULL;
  }
  time_t link_age = time(NULL) - link_stat.st_mtime;
  if (link_age < linq->debounce_seconds) {
    *retry_after_seconds = linq->debounce_seconds - link_age;
    free(link_name);
    return NULL;
  }

  struct stat target_stat;
  if (fstatat(linq->dirfd, link_name, &target_stat, 0) < 0 ||
      target_stat.st_mtime > link_stat.st_mtime) {
    if (unlinkat(linq->dirfd, link_name, 0) < 0) {
      *error_code = errno;
      free(link_name);
      return NULL;
    }
    ++linq->head_index;
    --linq->size;
    free(link_name);
    return pop_from_linq(linq, length_guess, retry_after_seconds, error_code);
  }

  size_t max_size = length_guess + 1;
  for (;;) {
    char *target = malloc(max_size);
    if (!target) {
      *error_code = errno;
      free(link_name);
      return NULL;
    }

    int length = readlinkat(linq->dirfd, link_name, target, max_size);

    if (length < 0) {
      *error_code = errno;
      free(link_name);
      free(target);
      return NULL;
    }
    if (length < max_size) {
      if (unlinkat(linq->dirfd, link_name, 0) < 0) {
        *error_code = errno;
        free(link_name);
        free(target);
        return NULL;
      }
      ++linq->head_index;
      --linq->size;
      free(link_name);
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
