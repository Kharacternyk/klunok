#include "linq.h"
#include "base16.h"
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
  size_t head_index;
  size_t size;
};

static int dot_filter(const struct dirent *dirent) {
  const char *name = dirent->d_name;
  return !(name[0] == '.' &&
           (name[1] == 0 || (name[1] == '.' && name[2] == 0)));
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

static struct linq *load_or_create_linq(const char *path, bool try_to_create,
                                        int *error_code, bool *is_corrupted) {
  struct dirent **entries;
  int entry_count = scandir(path, &entries, dot_filter, alphasort);
  if (entry_count < 0) {
    if (errno == ENOENT && try_to_create) {
      create_linq_path(path, error_code);
      if (*error_code) {
        return NULL;
      }
      return load_or_create_linq(path, false, error_code, is_corrupted);
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
    linq->head_index = decode_from_base16(entries[0]->d_name, is_corrupted);
    if (*is_corrupted) {
      free(linq);
      free_entries(entries, entry_count);
      return NULL;
    }
  } else {
    linq->head_index = 0;
  }

  linq->size = entry_count;

  linq->dirfd = open(path, O_DIRECTORY);
  if (linq->dirfd < 0) {
    *error_code = errno;
    free(linq);
    free_entries(entries, entry_count);
    return NULL;
  }

  return linq;
}

struct linq *load_linq(const char *path, int *error_code, bool *is_corrupted) {
  return load_or_create_linq(path, true, error_code, is_corrupted);
}

void push_to_linq(const char *path, struct linq *linq, int *error_code) {
  char *filename = encode_to_base16(linq->head_index + linq->size, error_code);
  if (*error_code) {
    return;
  }

  if (symlinkat(path, linq->dirfd, filename) < 0) {
    *error_code = errno;
  } else {
    ++linq->size;
  }

  free(filename);
}

char *pop_from_linq(struct linq *linq, size_t length_guess, bool *is_empty,
                    int *error_code) {
  if (!linq->size) {
    *is_empty = true;
    return NULL;
  }

  char *filename = encode_to_base16(linq->head_index, error_code);
  if (*error_code) {
    return NULL;
  }

  size_t max_size = length_guess + 1;
  for (;;) {
    char *path = malloc(max_size);
    if (!path) {
      *error_code = errno;
      free(filename);
      return NULL;
    }

    int length = readlinkat(linq->dirfd, filename, path, max_size);

    if (length < 0) {
      *error_code = errno;
      free(filename);
      free(path);
      return NULL;
    }
    if (length < max_size) {
      if (unlinkat(linq->dirfd, filename, 0) < 0) {
        *error_code = errno;
        free(filename);
        free(path);
        return NULL;
      }
      ++linq->head_index;
      --linq->size;
      free(filename);
      path[length] = 0;
      return path;
    }

    free(path);
    max_size *= 2;
  }
}

void free_linq(struct linq *linq) {
  if (linq) {
    close(linq->dirfd);
    free(linq);
  }
}
