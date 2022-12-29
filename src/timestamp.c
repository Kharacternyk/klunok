#include "timestamp.h"
#include <stdlib.h>
#include <time.h>

char *get_timestamp(const char *format, size_t length_guess,
                    struct callback *error_callback) {
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  if (!tm) {
    invoke_callback(error_callback);
    return NULL;
  }

  size_t timestamp_max_size = length_guess + 1;
  char *timestamp = malloc(timestamp_max_size);

  for (;;) {
    size_t timestamp_actual_length =
        strftime(timestamp, timestamp_max_size, format, tm);

    if (timestamp_actual_length < 0) {
      invoke_callback(error_callback);
      free(timestamp);
      return NULL;
    } else if (timestamp_actual_length > 0) {
      return timestamp;
    } else if (timestamp_max_size > length_guess + 1) {
      invoke_callback(error_callback);
      free(timestamp);
      return NULL;
    }

    timestamp_max_size *= 2;
    free(timestamp);
    timestamp = malloc(timestamp_max_size);
  }
}
