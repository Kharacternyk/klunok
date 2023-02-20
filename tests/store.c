#include "store.h"
#include "messages.h"
#include "trace.h"
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define STORE_ROOT "./store"
#define COPYIED_FILE TEST_ROOT "/meson.build"
#define MISSING_FILE TEST_ROOT "/foobar"
#define FILE_VERSION "v123"
#define FILE_COPY STORE_ROOT COPYIED_FILE "/" FILE_VERSION

#define LOG_FILE "log.txt"
#define CURSOR_NAME "cursor"
#define LOG_FILE_V1 "v1"
#define LOG_FILE_V2 "v2"
#define LOG_LINE_1 "1234\n"
#define LOG_LINE_2 "abcdefgh\n"

void test_store() {
  struct trace *trace = create_trace();

  assert(access(FILE_COPY, F_OK) != 0);

  copy_to_store(COPYIED_FILE, FILE_VERSION, STORE_ROOT, trace);
  assert(ok(trace));

  assert(!access(FILE_COPY, F_OK));

  copy_to_store(MISSING_FILE, FILE_VERSION, STORE_ROOT, trace);
  assert(catch_static(messages.store.copy.file_does_not_exist, trace));

  int fd = open(LOG_FILE, O_CREAT | O_EXCL | O_WRONLY, S_IRWXU);
  assert(fd >= 0);
  assert(write(fd, LOG_LINE_1, strlen(LOG_LINE_1)) == strlen(LOG_LINE_1));

  copy_delta_to_store(LOG_FILE, LOG_FILE_V1, CURSOR_NAME, STORE_ROOT, trace);
  assert(ok(trace));

  struct stat log_stat;
  assert(stat(STORE_ROOT "/" LOG_FILE "/" LOG_FILE_V1, &log_stat) >= 0);
  assert(log_stat.st_size == strlen(LOG_LINE_1));

  assert(write(fd, LOG_LINE_2, strlen(LOG_LINE_2)) == strlen(LOG_LINE_2));
  copy_delta_to_store(LOG_FILE, LOG_FILE_V2, CURSOR_NAME, STORE_ROOT, trace);
  assert(ok(trace));

  assert(stat(STORE_ROOT "/" LOG_FILE "/" LOG_FILE_V2, &log_stat) >= 0);
  assert(log_stat.st_size == strlen(LOG_LINE_2));

  free(trace);
}
