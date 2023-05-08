#include <stdlib.h>

struct trace;

size_t read_counter(const char *path, struct trace *trace);
void write_counter(const char *path, size_t counter, struct trace *trace);
