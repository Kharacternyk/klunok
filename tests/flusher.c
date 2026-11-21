#include "flusher.h"
#include "buffer.h"
#include "serialize.h"
#include "test-flusher.h"
#include "trace.h"
#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/pidfd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/xattr.h>
#include <unistd.h>

int set_flush_xattr(const char *path, uint8_t version, const char *boot_id,
                    uint64_t timestamp, uint8_t kind, uint64_t id,
                    uint64_t time) {
  uint8_t xattr[2 * sizeof(uint8_t) + BOOT_ID_SIZE + 3 * sizeof(uint64_t)];
  size_t i = 0;

  xattr[i++] = version;
  memcpy(xattr + i, boot_id, BOOT_ID_SIZE);
  i += BOOT_ID_SIZE;

  i += (write_u64(timestamp, xattr + i), sizeof timestamp);
  xattr[i++] = kind;
  i += (write_u64(id, xattr + i), sizeof id);
  i += (write_u64(time, xattr + i), sizeof time);

  assert(i == sizeof xattr);

  return setxattr(path, "user.klunok.flush", xattr, sizeof xattr, 0);
}

void read_boot_id(char *boot_id) {
  int fd = open("/proc/sys/kernel/random/boot_id", O_RDONLY);
  assert(fd >= 0);

  size_t total_read = 0;

  while (total_read < BOOT_ID_SIZE) {
    ssize_t iter_read =
        read(fd, boot_id + total_read, BOOT_ID_SIZE - total_read);
    assert(iter_read >= 0);

    if (!iter_read) {
      break;
    }

    total_read += iter_read;
  }

  assert(!close(fd));
}

int listen_for_acknowledgements(const char *path) {
  struct sockaddr_un address = {.sun_family = AF_UNIX};
  assert(strlen(path) < sizeof address.sun_path);
  strcpy(address.sun_path, path);
  unlink(path);
  int fd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
  assert(fd >= 0);
  assert(!bind(fd, (struct sockaddr *)&address, sizeof address));
  assert(!listen(fd, 1));
  return fd;
}

void assert_acknowledgement(int fd, uint64_t id) {
  uint8_t expected[sizeof(uint8_t) + sizeof id];
  expected[0] = 1;
  write_u64(id, expected + 1);

  uint8_t response[sizeof expected];
  assert(recv(fd, response, sizeof response, MSG_TRUNC | MSG_DONTWAIT) ==
         sizeof response);
  assert(!memcmp(response, expected, sizeof expected));
}

struct buffer *create_acknowledgement_path(struct trace *trace) {
  struct buffer *acknowledgement_path = create_buffer(trace);
  concat_string("/tmp/klunok.", acknowledgement_path, trace);
  concat_size(getpid(), acknowledgement_path, trace);
  return acknowledgement_path;
}

void test_flusher(struct trace *trace) {
  pid_t pid = getpid();
  int pidfd = pidfd_open(pid, 0);
  assert(pidfd >= 0);
  const char *path = "flushable";
  int fd = creat(path, 0644);
  assert(fd >= 0);
  assert(!close(fd));

  struct flusher *flusher = create_flusher(1, trace);
  assert(ok(trace));

  assert(!get_request(path, flusher, trace));
  assert(ok(trace));

  char boot_id[BOOT_ID_SIZE];
  read_boot_id(boot_id);

  char wrong_boot_id[BOOT_ID_SIZE];
  memcpy(wrong_boot_id, boot_id, sizeof wrong_boot_id);
  wrong_boot_id[0] = wrong_boot_id[0] == '0' ? '1' : '0';

  if (set_flush_xattr(path, 1, wrong_boot_id, UINT64_MAX - 2,
                      bypass_queue_with_timestamp, 11, 22)) {
    free_flusher(flusher);
    exit(77);
  }

  assert(!get_request(path, flusher, trace));
  assert(ok(trace));

  struct buffer *acknowledgement_path = create_acknowledgement_path(trace);
  assert(ok(trace));

  const char *acknowledgement_path_string =
      get_string(get_view(acknowledgement_path));

  int listener = listen_for_acknowledgements(acknowledgement_path_string);

  assert(!set_flush_xattr(path, 1, boot_id, UINT64_MAX - 2,
                          bypass_queue_with_timestamp, 11, 22));
  struct flush_request *request = get_request(path, flusher, trace);
  assert(ok(trace));
  assert(request);
  assert(get_time(request) == 22);

  acknowledge(request, pid, pidfd, flusher, trace);
  assert(ok(trace));
  int acknowledgement_fd = accept(listener, NULL, NULL);
  assert(acknowledgement_fd >= 0);
  assert_acknowledgement(acknowledgement_fd, 11);

  /* The same numeric PID with a different pidfd must miss both the cache
     and the peer check on the new connection. */
  int other_pidfd = pidfd_open(getppid(), 0);
  assert(other_pidfd >= 0);
  acknowledge(request, pid, other_pidfd, flusher, trace);
  assert(ok(trace));
  int rejected = accept(listener, NULL, NULL);
  assert(rejected >= 0);
  uint8_t byte;
  assert(recv(rejected, &byte, sizeof byte, MSG_DONTWAIT) == 0);
  assert(!close(rejected));
  assert(!close(other_pidfd));

  free(request);

  assert(!get_request(path, flusher, trace));
  assert(ok(trace));

  assert(!set_flush_xattr(path, 1, boot_id, UINT64_MAX - 1,
                          bypass_queue_with_timestamp, 33, 44));
  request = get_request(path, flusher, trace);
  assert(ok(trace));
  assert(request);
  assert(get_time(request) == 44);

  acknowledge(request, pid, pidfd, flusher, trace);
  assert(ok(trace));
  assert_acknowledgement(acknowledgement_fd, 33);

  free(request);

  assert(!set_flush_xattr(path, 2, boot_id, UINT64_MAX,
                          bypass_queue_with_timestamp, 55, 66));
  assert(!get_request(path, flusher, trace));
  assert(ok(trace));

  assert(!set_flush_xattr(path, 1, boot_id, UINT64_MAX,
                          bypass_queue_with_timestamp, 55, UINT64_MAX));
  assert(!get_request(path, flusher, trace));
  assert(ok(trace));

  assert(!set_flush_xattr(path, 1, boot_id, UINT64_MAX, 10, 55, 0));
  assert(!get_request(path, flusher, trace));
  assert(ok(trace));

  assert(!set_flush_xattr(path, 1, boot_id, UINT64_MAX, bypass_queue, 55, 0));
  request = get_request(path, flusher, trace);
  assert(ok(trace));
  assert(request);
  assert(get_time(request) == 0);
  free(request);

  assert(!close(acknowledgement_fd));
  assert(!close(listener));
  assert(!close(pidfd));
  assert(!unlink(acknowledgement_path_string));
  free_buffer(acknowledgement_path);

  free_flusher(flusher);
}
