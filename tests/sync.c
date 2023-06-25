#include "sync.h"
#include "messages.h"
#include "test-constants.h"
#include "trace.h"
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void create_file(const char *path) {
  int fd = creat(path, 0644);
  assert(fd >= 0 && !close(fd));
}

void test_sync(struct trace *trace) {
  const char *destination = "path/to/dest";
  const char *source = TEST_ROOT "/meson.build";

  assert(access(destination, F_OK) != 0);

  sync_file(destination, source, 0, trace);
  assert(ok(trace));

  assert(!access(destination, F_OK));

  try(trace);
  sync_file(destination, source, 0, trace);
  assert(catch_static(messages.sync.destination_already_exists, trace));
  finally(trace);

  try(trace);
  sync_file("won't-exist", TEST_ROOT "/I-do-not-exist", 0, trace);
  assert(catch_static(messages.sync.source_does_not_exist, trace));
  finally(trace);

  const char *log_destination = "path/to/log.txt";
  size_t offset = 12;
  size_t new_offset = sync_file(log_destination, source, offset, trace);
  assert(ok(trace));

  struct stat source_stat;
  assert(stat(source, &source_stat) >= 0);

  assert(new_offset == source_stat.st_size);

  struct stat log_destination_stat;
  assert(stat(log_destination, &log_destination_stat) >= 0);

  assert(new_offset == log_destination_stat.st_size + offset);

  assert(!mkdir("a", 0755));
  assert(!mkdir("a/b", 0755));
  create_file("a/b/c");
  create_file("a/b/d");
  create_file("a/b/e");

  assert(!mkdir("f", 0755));
  assert(!mkdir("f/b", 0755));
  create_file("f/b/c");
  create_file("f/b/e");

  assert(access("g", F_OK));

  sync_shallow_tree("g", "a", "f", trace);
  assert(ok(trace));
  assert(!access("g/b/c", F_OK));
  assert(access("g/b/d", F_OK));
  assert(!access("g/b/e", F_OK));
  assert(!access("a/b/c", F_OK));
  assert(access("a/b/d", F_OK));
  assert(!access("a/b/e", F_OK));
}
