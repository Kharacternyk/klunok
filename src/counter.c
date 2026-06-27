#include "counter.h"
#include "buffer.h"
#include "parents.h"
#include "trace.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

size_t read_counter(const char *path, struct trace *trace) {
  if (!ok(trace)) {
    return 0;
  }

  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    if (errno != ENOENT) {
      throw_errno(trace);
    }
    return 0;
  }

  char digits[21];
  size_t total_read = 0;

  while (total_read + 1 < sizeof digits) {
    size_t iter_read = TNEG(
        read(fd, digits + total_read, sizeof digits - total_read - 1), trace);

    if (!ok(trace)) {
      close(fd);
      return 0;
    }
    if (!iter_read) {
      break;
    }

    total_read += iter_read;
  }

  close(fd);
  digits[total_read] = 0;

  return strtoll(digits, NULL, 10);
}

void write_counter(const char *path, size_t counter, struct trace *trace) {
  if (ok(trace) && counter == 0) {
    if (!unlink(path)) {
      remove_empty_parents(path, trace);
    } else if (errno != ENOENT) {
      throw_errno(trace);
    }
    return;
  }

  create_parents(path, trace);
  int fd = TNEG(open(path, O_WRONLY | O_CREAT, 0644), trace);
  TNEG(ftruncate(fd, 0), trace);

  struct buffer *buffer = create_buffer(trace);
  concat_size(counter, buffer, trace);

  if (!ok(trace)) {
    free_buffer(buffer);
    if (fd >= 0) {
      close(fd);
    }
    return;
  };

  const struct buffer_view *view = get_view(buffer);
  size_t length = get_length(view);
  const char *string = get_string(view);

  for (size_t i = 0; ok(trace) && i < length;) {
    i += TNEG(write(fd, string + i, length - i), trace);
  }

  if (fd >= 0) {
    close(fd);
  }

  free_buffer(buffer);
}
