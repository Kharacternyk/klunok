#include <stdbool.h>
#include <stddef.h>
#include <time.h>

struct trace;

char *get_timestamp(time_t time, const char *format, size_t max_length,
                    struct trace *trace) __attribute__((warn_unused_result));
