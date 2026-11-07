#include "handler.h"
#include "buffer.h"
#include "sync.h"
#include "test-constants.h"
#include "test-flusher.h"
#include "trace.h"
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define CONFIG_BASE "lua/handler.lua"
#define CONFIG TEST_ROOT "/" CONFIG_BASE
#define F1 CONFIG
#define F2 TEST_ROOT "/lua/empty.lua"
#define EMPTY "empty"
#define FLUSHED "project/demo/flushed.txt"
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

  sync_file("flush.lua", TEST_ROOT "/lua/flush.lua", 0, trace);
  assert(ok(trace));
  assert(!chmod("flush.lua", 0600));
  char *cwd = getcwd(NULL, 0);
  assert(cwd);
  handler = load_handler("flush.lua", strlen(cwd) + 1, trace);
  free(cwd);
  assert(ok(trace));

  assert(!mkdir("project", 0700));
  assert(!mkdir("project/demo", 0700));
  fd = open(FLUSHED, O_CREAT | O_RDONLY, S_IRWXU);
  assert(fd >= 0);

  char boot_id[BOOT_ID_SIZE];
  read_boot_id(boot_id);

  if (set_flush_xattr(FLUSHED, 1, boot_id, UINT64_MAX - 1, 77, 22)) {
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

  assert(!access("klunok/store/" FLUSHED "/22.txt", F_OK));

  assert(!set_flush_xattr(FLUSHED, 1, boot_id, UINT64_MAX, 78, 0));
  handle_close_write(getpid(), fd, handler, trace);
  assert(ok(trace));
  assert_acknowledgement(acknowledgement_fd, 78);

  assert(!access("klunok/store/" FLUSHED "/0.txt", F_OK));

  /* A flushed config must reload before the queued project snapshot. */
  int config_fd = open("flush.lua", O_WRONLY | O_APPEND);
  assert(config_fd >= 0);
  const char change[] = "version_pattern = 'reloaded'\n";
  assert(write(config_fd, change, sizeof change - 1) == sizeof change - 1);
  assert(!set_flush_xattr("flush.lua", 1, boot_id, UINT64_MAX, 79, 0));
  handle_close_write(getpid(), config_fd, handler, trace);
  assert(ok(trace));
  assert_acknowledgement(acknowledgement_fd, 79);
  assert(!close(config_fd));

  assert(!close(acknowledgement_fd));
  assert(!unlink(acknowledgement_path_string));
  free_buffer(acknowledgement_path);
  close(fd);

  pause = handle_timeout(handler, trace);
  assert(ok(trace));
  assert(pause < 0);
  assert(!access("klunok/projects/demo/reloaded/flushed.txt", F_OK));

  free_handler(handler);
}
