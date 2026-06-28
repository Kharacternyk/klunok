#include "flusher.h"
#include "trace.h"
#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/xattr.h>
#include <unistd.h>

#define BOOT_ID_SIZE 36

static size_t write_u64(uint64_t value, uint8_t *destination) {
  for (size_t i = sizeof value; i; --i) {
    destination[i - 1] = value;
    value >>= 8;
  }

  return sizeof value;
}

static int set_flush_xattr(const char *path, uint8_t version,
                           const char *boot_id, uint64_t timestamp, uint64_t id,
                           uint64_t time) {
  uint8_t xattr[sizeof(uint8_t) + BOOT_ID_SIZE + 3 * sizeof(uint64_t)];
  size_t i = 0;

  xattr[i++] = version;
  memcpy(xattr + i, boot_id, BOOT_ID_SIZE);
  i += BOOT_ID_SIZE;
  i += write_u64(timestamp, xattr + i);
  i += write_u64(id, xattr + i);
  i += write_u64(time, xattr + i);
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

  set_flush_xattr(path, 1, wrong_boot_id, UINT64_MAX - 2, 11, 22);
  assert(!get_request(path, flusher, trace));
  assert(ok(trace));

  set_flush_xattr(path, 1, boot_id, UINT64_MAX - 2, 11, 22);
  struct flush_request *request = get_request(path, flusher, trace);
  assert(ok(trace));
  assert(request);
  assert(get_id(request) == 11);
  assert(get_time(request) == 22);
  free(request);

  assert(!get_request(path, flusher, trace));
  assert(ok(trace));

  set_flush_xattr(path, 1, boot_id, UINT64_MAX - 1, 33, 44);
  request = get_request(path, flusher, trace);
  assert(ok(trace));
  assert(request);
  assert(get_id(request) == 33);
  assert(get_time(request) == 44);
  free(request);

  set_flush_xattr(path, 2, boot_id, UINT64_MAX, 55, 66);
  assert(!get_request(path, flusher, trace));
  assert(ok(trace));

  free_flusher(flusher);
}
