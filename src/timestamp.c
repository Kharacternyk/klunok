#include "timestamp.h"
#include "messages.h"
#include "trace.h"
#include <stdlib.h>
#include <time.h>

char *get_timestamp(time_t time, const char *format, size_t max_length,
                    struct trace *trace) {
  struct tm *tm = TNULL(localtime(&time), trace);
  size_t max_size = max_length + 1;
  char *timestamp = TNULL(malloc(max_size), trace);

  if (!TNEG(strftime(timestamp, max_size, format, tm), trace)) {
    throw_static(messages.timestamp.overflow, trace);
  }
  if (!ok(trace)) {
    free(timestamp);
    return NULL;
  }

  return timestamp;
}
