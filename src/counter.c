#include "counter.h"
#include "buffer.h"
#include "parents.h"
#include "trace.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

/* FIXME avoid one-char IO */

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

  size_t counter = 0;

  for (;;) {
    char digit = 0;
    if (TNEG(read(fd, &digit, 1), trace) <= 0 || digit < '0' || digit > '9') {
      break;
    }
    counter *= 10;
    counter += digit - '0';
  }

  close(fd);

  return counter;
}

void write_counter(const char *path, size_t counter, struct trace *trace) {
  if (counter == 0) {
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

  for (size_t i = 0; ok(trace) && i < get_length(get_view(buffer)); ++i) {
    TNEG(write(fd, get_string(get_view(buffer)) + i, 1), trace);
  }

  if (fd >= 0) {
    close(fd);
  }

  free_buffer(buffer);
}
