#include "flusher.h"
#include "buffer.h"
#include "set.h"
#include "trace.h"
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/xattr.h>
#include <time.h>
#include <unistd.h>

struct flusher {
  char boot_id[36];
  uint8_t boot_id_size;
  uint64_t global_timestamp;
  struct set *path_timestamps;
};

struct flusher *create_flusher(size_t flushed_path_count_guess,
                               struct trace *trace) {
  struct flusher *flusher = TNULL(malloc(sizeof(struct flusher)), trace);
  struct set *path_timestamps =
      TNULL(create_set(flushed_path_count_guess, trace), trace);
  struct timespec timespec;

  TNEG(clock_gettime(CLOCK_MONOTONIC, &timespec), trace);

  int fd = TNEG(open("/proc/sys/kernel/random/boot_id", O_RDONLY), trace);

  if (!ok(trace)) {
    free(flusher);
    free_set(path_timestamps);
    return NULL;
  }

  flusher->global_timestamp = timespec.tv_nsec + 1000000000 * timespec.tv_sec;
  flusher->path_timestamps = path_timestamps;

  size_t total_read = 0;

  while (total_read < sizeof flusher->boot_id) {
    ssize_t iter_read = TNEG(read(fd, flusher->boot_id + total_read,
                                  sizeof flusher->boot_id - total_read),
                             trace);

    if (!ok(trace)) {
      free(flusher);
      free_set(path_timestamps);
      close(fd);
      return NULL;
    }

    if (!iter_read) {
      break;
    }

    total_read += iter_read;
  }

  flusher->boot_id_size = total_read;

  TNEG(close(fd), trace);

  if (!ok(trace)) {
    free(flusher);
    free_set(path_timestamps);
    return NULL;
  }

  return flusher;
}

bool should_flush(const char *path, char *action_destination,
                  size_t action_destination_size, struct flusher *flusher,
                  struct trace *trace) {
  if (!ok(trace)) {
    return false;
  }

  assert(action_destination_size);

  char boot_id[sizeof flusher->boot_id];
  ssize_t boot_id_size =
      getxattr(path, "user.klunok.flush.boot_id", boot_id, sizeof boot_id);

  if (boot_id_size != flusher->boot_id_size ||
      memcmp(boot_id, flusher->boot_id, boot_id_size)) {
    return false;
  }

  char timestamp[32];
  ssize_t timestamp_size = getxattr(path, "user.klunok.flush.timestamp",
                                    timestamp, sizeof timestamp - 1);
  if (timestamp_size < 0) {
    return false;
  }

  timestamp[timestamp_size] = 0;

  uint64_t timestamp_value = strtoull(timestamp, NULL, 10);

  if (timestamp_value <= flusher->global_timestamp) {
    return false;
  }

  struct buffer_view *path_view = create_buffer_view(path, trace);

  if (!ok(trace)) {
    return false;
  }

  uint64_t last_timestamp =
      get_last_metadata(path_view, flusher->path_timestamps);
  free_buffer_view(path_view);

  if (timestamp_value <= last_timestamp) {
    return false;
  }

  ssize_t action_size =
      getxattr(path, "user.klunok.flush.action", action_destination,
               action_destination_size - 1);

  if (action_size < 0) {
    return false;
  }

  action_destination[action_size] = 0;
  add_with_metadata(path, timestamp_value, flusher->path_timestamps, trace);

  return ok(trace);
}

void free_flusher(struct flusher *flusher) {
  if (flusher) {
    free_set(flusher->path_timestamps);
    free(flusher);
  }
}
