#include <stdlib.h>

struct trace;

size_t read_counter(const char *path, struct trace *trace)
    __attribute__((warn_unused_result));
void write_counter(const char *path, size_t counter, struct trace *trace);
