#include "flusher.h"
#include "buffer.h"
#include "process.h"
#include "serialize.h"
#include "set.h"
#include "trace.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/xattr.h>
#include <time.h>
#include <unistd.h>

struct flusher {
  char boot_id[36];
  uint8_t boot_id_size;
  uint64_t global_timestamp;
  struct set *path_timestamps;

  int peer_pidfd;
  int socket_fd;
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

  flusher->peer_pidfd = -1;
  flusher->socket_fd = -1;
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
  enum flush_kind kind;
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
                     sizeof xattr.timestamp + sizeof(uint8_t) +
                     sizeof xattr.request.id + sizeof xattr.request.time];

  ssize_t wire_xattr_size =
      getxattr(path, "user.klunok.flush", wire_xattr, sizeof wire_xattr);

  if (wire_xattr_size != (ssize_t)sizeof wire_xattr) {
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

  enum flush_kind kind = wire_xattr[i++];

  if (kind != bypass_queue && kind != bypass_queue_with_timestamp &&
      kind != add_to_queue && kind != drop_from_queue) {
    return NULL;
  }

  xattr.request.kind = kind;

  for (size_t k = i; i < k + sizeof xattr.request.id; ++i) {
    xattr.request.id <<= 8;
    xattr.request.id += wire_xattr[i];
  }
  for (size_t k = i; i < k + sizeof xattr.request.time; ++i) {
    xattr.request.time <<= 8;
    xattr.request.time += wire_xattr[i];
  }

  if (kind == bypass_queue_with_timestamp) {
    time_t request_time = xattr.request.time;

    if (request_time < 0 || (uint64_t)request_time != xattr.request.time ||
        !localtime(&request_time)) {
      return NULL;
    }
  } else if (xattr.request.time != 0) {
    return NULL;
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

enum flush_kind get_kind(struct flush_request *request) {
  return request->kind;
}

uint64_t get_time(struct flush_request *request) { return request->time; }

static void disconnect_peer(struct flusher *flusher) {
  if (flusher->socket_fd >= 0) {
    close(flusher->socket_fd);
    close(flusher->peer_pidfd);
    flusher->socket_fd = -1;
    flusher->peer_pidfd = -1;
  }
}

static bool connect_peer(pid_t pid, int pidfd, struct flusher *flusher,
                         struct trace *trace) {
  struct buffer *path = create_buffer(trace);
  concat_string("/tmp/klunok.", path, trace);
  concat_size(pid, path, trace);
  struct sockaddr_un address = {.sun_family = AF_UNIX};
  if (!ok(trace) || get_length(get_view(path)) >= sizeof address.sun_path) {
    free_buffer(path);
    return false;
  }
  strcpy(address.sun_path, get_string(get_view(path)));
  free_buffer(path);

  int fd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    return false;
  }
  int peer = -1;
  socklen_t size = sizeof peer;
  if (connect(fd, (struct sockaddr *)&address, sizeof address) < 0 ||
      getsockopt(fd, SOL_SOCKET, SO_PEERPIDFD, &peer, &size) < 0 ||
      !same_process(pidfd, peer)) {
    if (peer >= 0) {
      close(peer);
    }
    close(fd);
    return false;
  }
  disconnect_peer(flusher);
  flusher->socket_fd = fd;
  flusher->peer_pidfd = peer;
  return true;
}

void acknowledge(const struct flush_request *request, pid_t pid, int pidfd,
                 struct flusher *flusher, struct trace *trace) {
  if (!ok(trace) || pidfd < 0) {
    return;
  }
  uint8_t response[9];
  response[0] = 1;
  write_u64(request->id, response + 1);

  for (int attempts_left = same_process(pidfd, flusher->peer_pidfd) ? 2 : 1;
       attempts_left > 0; --attempts_left) {
    if (attempts_left == 1 && !connect_peer(pid, pidfd, flusher, trace)) {
      return;
    }
    ssize_t sent =
        send(flusher->socket_fd, response, sizeof response, MSG_NOSIGNAL);
    if (sent >= 0 || errno == EAGAIN || errno == EINTR) {
      return;
    }
    disconnect_peer(flusher);
  }
}

void free_flusher(struct flusher *flusher) {
  if (flusher) {
    disconnect_peer(flusher);
    free_set(flusher->path_timestamps);
    free(flusher);
  }
}
