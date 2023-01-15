#include "trace.h"
#include <stdbool.h>
#include <stddef.h>

char *get_timestamp(const char *format, size_t max_length, struct trace *trace);
