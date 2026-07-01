#include "flusher.h"
#include "buffer.h"
#include "serialize.h"
#include "set.h"
#include "trace.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <time.h>
#include <unistd.h>

struct flusher {
  char boot_id[36];
  uint8_t boot_id_size;
  uint64_t global_timestamp;
  struct set *path_timestamps;

  pid_t last_pid;
  int last_pid_fd;
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

  flusher->last_pid = 0;
  flusher->last_pid_fd = -1;
  flusher->global_timestamp =
      timespec.tv_nsec + 1000000000 * (uint64_t)timespec.tv_sec;
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

  close(fd);

  return flusher;
}

struct flush_request {
  uint64_t id;
  uint64_t time;
};

struct flush_xattr {
  uint8_t version;
  char boot_id[36];
  uint64_t timestamp;
  struct flush_request request;
};

struct flush_request *get_request(const char *path, struct flusher *flusher,
                                  struct trace *trace) {
  if (!ok(trace)) {
    return NULL;
  }

  struct flush_xattr xattr = {};
  uint8_t wire_xattr[sizeof xattr.version + sizeof xattr.boot_id +
                     sizeof xattr.timestamp + sizeof xattr.request.id +
                     sizeof xattr.request.time];

  ssize_t wire_xattr_size =
      getxattr(path, "user.klunok.flush", wire_xattr, sizeof wire_xattr);

  if (wire_xattr_size < 0 || wire_xattr_size < sizeof wire_xattr) {
    return NULL;
  }

  size_t i = 0;
  xattr.version = wire_xattr[i++];

  if (xattr.version != 1) {
    return NULL;
  }

  for (size_t k = i; i < k + sizeof xattr.boot_id; ++i) {
    xattr.boot_id[i - k] = wire_xattr[i];
  }
  if (memcmp(xattr.boot_id, flusher->boot_id, flusher->boot_id_size)) {
    return NULL;
  }

  for (size_t k = i; i < k + sizeof xattr.timestamp; ++i) {
    xattr.timestamp <<= 8;
    xattr.timestamp += wire_xattr[i];
  }
  if (xattr.timestamp < flusher->global_timestamp) {
    return NULL;
  }

  struct buffer_view *path_view = create_buffer_view(path, trace);

  if (!ok(trace)) {
    return NULL;
  }

  uint64_t last_timestamp =
      get_last_metadata(path_view, flusher->path_timestamps);
  free_buffer_view(path_view);

  if (xattr.timestamp <= last_timestamp) {
    return NULL;
  }

  for (size_t k = i; i < k + sizeof xattr.request.id; ++i) {
    xattr.request.id <<= 8;
    xattr.request.id += wire_xattr[i];
  }
  for (size_t k = i; i < k + sizeof xattr.request.time; ++i) {
    xattr.request.time <<= 8;
    xattr.request.time += wire_xattr[i];
  }

  add_with_metadata(path, xattr.timestamp, flusher->path_timestamps, trace);
  struct flush_request *request =
      TNULL(malloc(sizeof(struct flush_request)), trace);

  if (!ok(trace)) {
    return NULL;
  }

  *request = xattr.request;

  return request;
}

uint64_t get_id(struct flush_request *request) { return request->id; }
uint64_t get_time(struct flush_request *request) { return request->time; }

void acknowledge(const struct flush_request *request, pid_t pid,
                 struct flusher *flusher, struct trace *trace) {
  if (!ok(trace)) {
    return;
  }

  if (pid != flusher->last_pid) {
    struct buffer *path = create_buffer(trace);
    concat_string("/tmp/klunok.", path, trace);
    concat_size(pid, path, trace);

    if (!ok(trace)) {
      free_buffer(path);
      return;
    }

    int fd = open(get_string(get_view(path)), O_WRONLY | O_NONBLOCK);

    free_buffer(path);

    if (fd < 0) {
      return;
    }

    struct stat stat;

    if (fstat(fd, &stat) || !S_ISFIFO(stat.st_mode)) {
      close(fd);
      return;
    }

    if (flusher->last_pid_fd >= 0) {
      close(flusher->last_pid_fd);
    }

    flusher->last_pid = pid;
    flusher->last_pid_fd = fd;
  }

  uint8_t response[9];
  response[0] = 1;
  write_u64(request->id, response + 1);

  /* No looping here, sizeof id is less than PIPE_BUF */
  if (write(flusher->last_pid_fd, &response, sizeof response) < 0 &&
      errno == EPIPE) {
    flusher->last_pid = 0;

    if (flusher->last_pid_fd >= 0) {
      close(flusher->last_pid_fd);
      flusher->last_pid_fd = -1;
    }
  }
}

void free_flusher(struct flusher *flusher) {
  if (flusher) {
    if (flusher->last_pid_fd >= 0) {
      close(flusher->last_pid_fd);
    }

    free_set(flusher->path_timestamps);
    free(flusher);
  }
}
