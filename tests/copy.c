#include "copy.h"
#include "messages.h"
#include "test-constants.h"
#include "trace.h"
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void test_copy(struct trace *trace) {
  const char *destination = "path/to/dest";
  const char *source = TEST_ROOT "/meson.build";

  assert(access(destination, F_OK) != 0);

  copy_file(destination, source, 0, trace);
  assert(ok(trace));

  assert(!access(destination, F_OK));

  try(trace);
  copy_file(destination, source, 0, trace);
  assert(catch_static(messages.copy.destination_already_exists, trace));
  finally(trace);

  try(trace);
  copy_file("won't-exist", TEST_ROOT "/I-do-not-exist", 0, trace);
  assert(catch_static(messages.copy.source_does_not_exist, trace));
  finally(trace);

  const char *log_destination = "path/to/log.txt";
  size_t offset = 12;
  size_t new_offset = copy_file(log_destination, source, offset, trace);
  assert(ok(trace));

  struct stat source_stat;
  assert(stat(source, &source_stat) >= 0);

  assert(new_offset == source_stat.st_size);

  struct stat log_destination_stat;
  assert(stat(log_destination, &log_destination_stat) >= 0);

  assert(new_offset == log_destination_stat.st_size + offset);
}
