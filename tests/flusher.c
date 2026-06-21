#include "flusher.h"
#include "trace.h"
#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/xattr.h>
#include <unistd.h>

#define TIMESTAMP "1844674407370955161"
#define BOOT_ID_SIZE 36

static size_t read_boot_id(char boot_id[BOOT_ID_SIZE]) {
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

  return total_read;
}

void test_flusher(struct trace *trace) {
  const char *path = "flushable";
  int fd = creat(path, 0644);
  assert(fd >= 0);
  assert(!close(fd));

  static const char *name = "user.klunok.test";
  if (setxattr(path, name, "", 0, 0) || removexattr(path, name)) {
    exit(77);
  }

  struct flusher *flusher = create_flusher(1, trace);
  assert(ok(trace));

  assert(!should_flush(path, flusher, trace));
  assert(ok(trace));

  char boot_id[BOOT_ID_SIZE];
  size_t boot_id_size = read_boot_id(boot_id);
  assert(
      !setxattr(path, "user.klunok.flush.boot_id", boot_id, boot_id_size, 0));

  const char *timestamp_attribute = "user.klunok.flush.timestamp";
  assert(!setxattr(path, timestamp_attribute, TIMESTAMP "0",
                   strlen(TIMESTAMP "0"), 0));

  assert(should_flush(path, flusher, trace));
  assert(ok(trace));

  assert(!should_flush(path, flusher, trace));
  assert(ok(trace));

  assert(!setxattr(path, timestamp_attribute, TIMESTAMP "1",
                   strlen(TIMESTAMP "1"), 0));

  assert(should_flush(path, flusher, trace));
  assert(ok(trace));

  assert(!setxattr(path, timestamp_attribute, TIMESTAMP "2",
                   strlen(TIMESTAMP "2"), 0));

  boot_id[0] = boot_id[0] == '0' ? '1' : '0';
  assert(
      !setxattr(path, "user.klunok.flush.boot_id", boot_id, boot_id_size, 0));
  assert(!should_flush(path, flusher, trace));
  assert(ok(trace));

  free_flusher(flusher);
}
