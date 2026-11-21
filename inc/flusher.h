#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

enum flush_kind {
  bypass_queue,
  bypass_queue_with_timestamp,
  add_to_queue,
  drop_from_queue,
};

struct trace;

struct flusher *create_flusher(const char *fifo_prefix,
                               size_t flushed_path_count_guess,
                               struct trace *trace)
    __attribute__((warn_unused_result));

struct flush_request *get_request(const char *path, struct flusher *flusher,
                                  struct trace *trace)
    __attribute__((warn_unused_result));

enum flush_kind get_kind(struct flush_request *request)
    __attribute__((pure, nonnull, warn_unused_result));

uint64_t get_time(struct flush_request *request)
    __attribute__((pure, nonnull, warn_unused_result));

void acknowledge(const struct flush_request *request, pid_t pid,
                 struct flusher *flusher, struct trace *trace);

void free_flusher(struct flusher *flusher);
