#include "timestamp.h"
#include "messages.h"
#include <stdlib.h>
#include <time.h>

char *get_timestamp(const char *format, size_t max_length,
                    struct trace *trace) {
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  if (!tm) {
    trace_errno(trace);
    return NULL;
  }

  size_t max_size = max_length + 1;
  char *timestamp = malloc(max_size);
  size_t actual_length = strftime(timestamp, max_size, format, tm);

  if (actual_length <= 0) {
    if (actual_length == 0) {
      trace_static(messages.timestamp.overflow, trace);
    }
    free(timestamp);
    return NULL;
  }

  return timestamp;
}
