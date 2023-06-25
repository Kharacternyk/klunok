#include <unistd.h>

struct trace;

off_t copy_file(const char *destination, const char *source,
                off_t source_offset, struct trace *trace);
void copy_shallow_tree(const char *destination, const char *source,
                       const char *existence_filter_root, struct trace *trace);
