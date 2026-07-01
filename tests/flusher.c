#include "flusher.h"
#include "buffer.h"
#include "serialize.h"
#include "trace.h"
#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <unistd.h>

#define BOOT_ID_SIZE 36

static int set_flush_xattr(const char *path, uint8_t version,
                           const char *boot_id, uint64_t timestamp, uint64_t id,
                           uint64_t time) {
  uint8_t xattr[sizeof(uint8_t) + BOOT_ID_SIZE + 3 * sizeof(uint64_t)];
  size_t i = 0;

  xattr[i++] = version;
  memcpy(xattr + i, boot_id, BOOT_ID_SIZE);
  i += BOOT_ID_SIZE;

  i += (write_u64(timestamp, xattr + i), sizeof timestamp);
  i += (write_u64(id, xattr + i), sizeof id);
  i += (write_u64(time, xattr + i), sizeof time);

  assert(i == sizeof xattr);

  return setxattr(path, "user.klunok.flush", xattr, sizeof xattr, 0);
}

static void read_boot_id(char *boot_id) {
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

static void assert_acknowledgement(int fd) {
  uint8_t response[20];
  assert(read(fd, response, sizeof response) == 9);
  assert(response[0] == 1);
}

void test_flusher(struct trace *trace) {
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

  if (set_flush_xattr(path, 1, wrong_boot_id, UINT64_MAX - 2, 11, 22)) {
    exit(77);
  }

  assert(!get_request(path, flusher, trace));
  assert(ok(trace));

  struct buffer *acknowledgement_path = create_buffer(trace);
  concat_string("/tmp/klunok.", acknowledgement_path, trace);
  concat_size(getpid(), acknowledgement_path, trace);
  assert(ok(trace));

  const char *acknowledgement_path_string =
      get_string(get_view(acknowledgement_path));

  assert(!mkfifo(acknowledgement_path_string, 0600));

  int acknowledgement_fd =
      open(acknowledgement_path_string, O_RDONLY | O_NONBLOCK);
  assert(acknowledgement_fd >= 0);

  assert(!set_flush_xattr(path, 1, boot_id, UINT64_MAX - 2, 11, 22));
  struct flush_request *request = get_request(path, flusher, trace);
  assert(ok(trace));
  assert(request);
  assert(get_time(request) == 22);

  acknowledge(request, getpid(), flusher, trace);
  assert(ok(trace));
  assert_acknowledgement(acknowledgement_fd);

  free(request);

  assert(!get_request(path, flusher, trace));
  assert(ok(trace));

  assert(!set_flush_xattr(path, 1, boot_id, UINT64_MAX - 1, 33, 44));
  request = get_request(path, flusher, trace);
  assert(ok(trace));
  assert(request);
  assert(get_time(request) == 44);

  acknowledge(request, getpid(), flusher, trace);
  assert(ok(trace));
  assert_acknowledgement(acknowledgement_fd);

  free(request);

  assert(!set_flush_xattr(path, 2, boot_id, UINT64_MAX, 55, 66));
  assert(!get_request(path, flusher, trace));
  assert(ok(trace));

  assert(!close(acknowledgement_fd));
  assert(!unlink(acknowledgement_path_string));
  free_buffer(acknowledgement_path);

  free_flusher(flusher);
}
