#include <stdbool.h>
#include <stddef.h>

struct trace;

struct flusher *create_flusher(size_t flushed_path_count_guess,
                               struct trace *trace)
    __attribute__((warn_unused_result));
bool should_flush(const char *path, struct flusher *flusher,
                  struct trace *trace);
void free_flusher(struct flusher *flusher);
