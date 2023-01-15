#include "trace.h"
#include <fcntl.h>

void create_parents(const char *path, mode_t mode, struct trace *trace);
void remove_empty_parents(const char *path, struct trace *trace);
