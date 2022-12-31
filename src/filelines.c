#include "filelines.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

struct set *get_lines(const char *path, size_t average_line_length_guess,
                      struct callback *error_callback) {
  struct stat path_stat;
  if (stat(path, &path_stat) < 0) {
    invoke_callback(error_callback);
    return NULL;
  }
  size_t size_guess = path_stat.st_size / (average_line_length_guess + 1);

  bool is_error = false;
  struct callback *wrapped_error_callback =
      create_simple_callback(&is_error, error_callback);
  if (!wrapped_error_callback) {
    invoke_callback(error_callback);
    return NULL;
  }

  struct set *set = create_set(size_guess, wrapped_error_callback);
  if (is_error) {
    goto callback_cleanup;
  }

  FILE *file = fopen(path, "r");
  if (!file) {
    goto set_cleanup;
  }

  for (;;) {
    char *line = NULL;
    size_t size;
    ssize_t length = getline(&line, &size, file);

    if (length < 0) {
      if (ferror(file)) {
        invoke_callback(error_callback);
        free(line);
        goto file_cleanup;
      }
      free(line);
      free(wrapped_error_callback);
      fclose(file);
      return set;
    }

    line[length - 1] = 0;
    add_to_set(line, set, wrapped_error_callback);
    if (is_error) {
      goto file_cleanup;
    }
    line = NULL;
  }

file_cleanup:
  fclose(file);
set_cleanup:
  free_set(set);
callback_cleanup:
  free(wrapped_error_callback);

  return NULL;
}
