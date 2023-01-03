#include "timestamp.h"
#include <stdlib.h>
#include <time.h>

char *get_timestamp(const char *format, size_t max_length,
                    const struct callback *error_callback, bool *is_overflow) {
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  if (!tm) {
    invoke_callback(error_callback);
    return NULL;
  }

  size_t max_size = max_length + 1;
  char *timestamp = malloc(max_size);
  size_t actual_length = strftime(timestamp, max_size, format, tm);

  if (actual_length <= 0) {
    if (actual_length == 0) {
      *is_overflow = true;
    }
    invoke_callback(error_callback);
    free(timestamp);
    return NULL;
  }

  return timestamp;
}
