#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

struct trace;

struct flusher *create_flusher(size_t flushed_path_count_guess,
                               struct trace *trace)
    __attribute__((warn_unused_result));

struct flush_request *get_request(const char *path, struct flusher *flusher,
                                  struct trace *trace)
    __attribute__((warn_unused_result));
uint64_t get_id(struct flush_request *request)
    __attribute__((pure, nonnull, warn_unused_result));
uint64_t get_time(struct flush_request *request)
    __attribute__((pure, nonnull, warn_unused_result));

void acknowledge_flush(uint64_t id, pid_t pid, struct flusher *flusher,
                       struct trace *trace);

void free_flusher(struct flusher *flusher);
