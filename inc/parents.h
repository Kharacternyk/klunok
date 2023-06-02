#include <stddef.h>

struct trace;

void create_parents(const char *path, struct trace *trace);
void remove_empty_parents(const char *path, struct trace *trace);
size_t get_common_parent_path_length(const char *first, const char *second)
    __attribute__((pure, warn_unused_result));
