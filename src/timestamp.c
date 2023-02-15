#include "timestamp.h"
#include "messages.h"
#include "trace.h"
#include <stdlib.h>
#include <time.h>

char *get_timestamp(const char *format, size_t max_length,
                    struct trace *trace) {
  time_t t = time(NULL);
  struct tm *tm = TNULL(localtime(&t), trace);
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
