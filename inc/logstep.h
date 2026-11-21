#include <stddef.h>

void write_full(int fd, const char *data, size_t size);
void logstep(int fd, const char *prefix, const char *message, size_t depth);
