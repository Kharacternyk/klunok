#include <stdbool.h>
#include <stddef.h>

struct trace;

char *get_timestamp(const char *format, size_t max_length, struct trace *trace)
    __attribute__((warn_unused_result));
