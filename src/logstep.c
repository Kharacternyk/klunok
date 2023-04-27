#include "logstep.h"
#include <string.h>
#include <unistd.h>

static void write_full(int fd, const char *data, size_t size) {
  while (size) {
    ssize_t written_size = write(fd, data, size);
    if (written_size <= 0) {
      return;
    }
    data += written_size;
    size -= written_size;
  }
}

void logstep(int fd, const char *prefix, const char *message, size_t depth) {
  for (size_t i = 1; i < depth; ++i) {
    write_full(fd, "  ", 2);
  }
  if (depth) {
    write_full(fd, "└─", 6);
  }
  if (prefix) {
    if (depth) {
      write_full(fd, "┤", 3);
    } else {
      write_full(fd, "│", 3);
    }
    write_full(fd, prefix, strlen(prefix));
    write_full(fd, "│", 3);
  }
  if (message) {
    if (prefix) {
      write_full(fd, " ", 1);
    }
    write_full(fd, message, strlen(message));
  }
  write_full(fd, "\n", 1);
}
