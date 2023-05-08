#define _GNU_SOURCE
#include "logstep.h"
#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <unistd.h>

static void assert_fd_content(int fd, const char *content) {
  char c;
  assert(!lseek(fd, 0, SEEK_SET));
  for (const char *cursor = content; *cursor; ++cursor) {
    assert(read(fd, &c, 1) == 1);
    assert(c == *cursor);
  }
  assert(!read(fd, &c, 1));
  assert(!ftruncate(fd, 0));
  assert(!lseek(fd, 0, SEEK_SET));
}

void test_logstep(void *unused) {
  int fd = memfd_create("", O_RDWR);
  logstep(fd, "prefix", "message", 0);
  assert_fd_content(fd, "│prefix│ message\n");
  logstep(fd, "prefix", "message", 1);
  assert_fd_content(fd, "└─┤prefix│ message\n");
  logstep(fd, "prefix", "message", 2);
  assert_fd_content(fd, "  └─┤prefix│ message\n");
  logstep(fd, "prefix", "message", 3);
  assert_fd_content(fd, "    └─┤prefix│ message\n");
  logstep(fd, "prefix", NULL, 3);
  assert_fd_content(fd, "    └─┤prefix│\n");
}
