#include "handler.h"
#include "buffer.h"
#include "test-constants.h"
#include "test-flusher.h"
#include "trace.h"
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define CONFIG_BASE "lua/handler.lua"
#define CONFIG TEST_ROOT "/" CONFIG_BASE
#define F1 CONFIG
#define F2 TEST_ROOT "/lua/empty.lua"
#define EMPTY "empty"
#define FLUSHED "flushed.txt"
#define IN_STORE(PATH) "klunok/store/" PATH "/version"

void test_handler(struct trace *trace) {
  struct handler *handler = load_handler(CONFIG, 1, trace);
  assert(ok(trace));

  time_t pause = handle_timeout(handler, trace);
  assert(ok(trace));
  assert(pause < 0);

  int fd = open(CONFIG, O_RDONLY);
  assert(fd >= 0);
  handle_open_exec(getpid(), fd, handler, trace);
  assert(ok(trace));

  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));

  close(fd);
  fd = open(F2, O_RDONLY);
  assert(fd >= 0);
  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));

  pause = handle_timeout(handler, trace);
  assert(ok(trace));
  assert(pause < 0);

  assert(access(IN_STORE(F1) ".lua", F_OK) == 0);
  assert(access(IN_STORE(F2) ".lua", F_OK) == 0);

  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));

  pause = handle_timeout(handler, trace);
  assert(ok(trace));
  assert(pause < 0);

  assert(access(IN_STORE(F2) "-1.lua", F_OK) == 0);

  close(fd);
  fd = open(EMPTY, O_CREAT | O_RDONLY, S_IRWXU);

  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));

  assert(unlink(EMPTY) >= 0);

  pause = handle_timeout(handler, trace);
  assert(ok(trace));
  assert(pause < 0);
  assert(access(IN_STORE(EMPTY), F_OK) != 0);

  close(fd);
  fd = open(EMPTY, O_CREAT | O_RDONLY, S_IWOTH);

  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));

  pause = handle_timeout(handler, trace);
  assert(ok(trace));
  assert(pause < 0);
  assert(access(IN_STORE(EMPTY), F_OK) != 0);

  close(fd);
  free_handler(handler);

  handler = load_handler(CONFIG, sizeof TEST_ROOT - 1, trace);
  assert(ok(trace));
  fd = open(CONFIG, O_RDONLY);
  assert(fd >= 0);
  handle_open_exec(getpid(), fd, handler, trace);
  assert(ok(trace));
  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));

  pause = handle_timeout(handler, trace);
  assert(pause < 0);
  assert(ok(trace));
  assert(access(IN_STORE(CONFIG_BASE) ".lua", F_OK) == 0);
  assert(access("klunok/projects/lua/version/handler.lua", F_OK) == 0);

  close(fd);
  free_handler(handler);

  handler = load_handler(CONFIG, 1, trace);
  assert(ok(trace));

  fd = open(FLUSHED, O_CREAT | O_RDONLY, S_IRWXU);
  assert(fd >= 0);

  char boot_id[BOOT_ID_SIZE];
  read_boot_id(boot_id);

  if (set_flush_xattr(FLUSHED, 1, boot_id, UINT64_MAX, 77, 88)) {
    exit(77);
  }

  struct buffer *acknowledgement_path = create_acknowledgement_path(trace);
  assert(ok(trace));

  const char *acknowledgement_path_string =
      get_string(get_view(acknowledgement_path));

  unlink(acknowledgement_path_string);
  assert(!mkfifo(acknowledgement_path_string, 0600));

  int acknowledgement_fd =
      open(acknowledgement_path_string, O_RDONLY | O_NONBLOCK);
  assert(acknowledgement_fd >= 0);

  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));
  assert_acknowledgement(acknowledgement_fd, 77);
  assert(!close(acknowledgement_fd));
  assert(!unlink(acknowledgement_path_string));
  free_buffer(acknowledgement_path);
  close(fd);

  struct buffer *flushed_store_path = create_buffer(trace);
  concat_string("klunok/store", flushed_store_path, trace);
  assert(ok(trace));

  char *cwd = getcwd(NULL, 0);
  assert(cwd);
  concat_string(cwd, flushed_store_path, trace);
  free(cwd);

  concat_char('/', flushed_store_path, trace);
  concat_string(FLUSHED, flushed_store_path, trace);
  concat_string("/version.txt", flushed_store_path, trace);
  assert(ok(trace));

  assert(access(get_string(get_view(flushed_store_path)), F_OK) == 0);
  free_buffer(flushed_store_path);

  pause = handle_timeout(handler, trace);
  assert(ok(trace));
  assert(pause < 0);

  free_handler(handler);
}
