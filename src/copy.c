#include "copy.h"
#include "messages.h"
#include "parents.h"
#include "trace.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

static void cleanup(const char *destination) {
  /*FIXME cleanup error reporting*/
  struct trace *cleanup_trace = create_trace();
  if (cleanup_trace) {
    remove_empty_parents(destination, cleanup_trace);
    catch_all(cleanup_trace);
    free(cleanup_trace);
  }
}

off_t copy_file(const char *destination, const char *source,
                off_t source_offset, struct trace *trace) {
  if (!ok(trace)) {
    return 0;
  }

  create_parents(destination, trace);
  if (!ok(trace)) {
    cleanup(destination);
    return 0;
  }

  int in_fd = open(source, O_RDONLY);
  if (in_fd < 0) {
    if (errno == ENOENT) {
      throw_static(messages.copy.source_does_not_exist, trace);
    } else if (errno == EACCES) {
      throw_static(messages.copy.source_permission_denied, trace);
    } else {
      throw_errno(trace);
    }
    cleanup(destination);
    return 0;
  }

  int out_fd = open(destination, O_CREAT | O_WRONLY | O_EXCL, 0444);
  if (out_fd < 0) {
    if (errno == EEXIST) {
      throw_static(messages.copy.destination_already_exists, trace);
    } else {
      throw_errno(trace);
    }
    close(in_fd);
    cleanup(destination);
    return 0;
  }

  struct stat in_fd_stat;
  if (fstat(in_fd, &in_fd_stat) < 0) {
    throw_errno(trace);
    close(out_fd);
    close(in_fd);
    cleanup(destination);
    return 0;
  }

  while (in_fd_stat.st_size) {
    size_t written_size =
        sendfile(out_fd, in_fd, &source_offset, in_fd_stat.st_size);
    if (written_size > 0) {
      in_fd_stat.st_size -= written_size;
    } else if (written_size == 0) {
      break;
    } else {
      throw_errno(trace);
      close(out_fd);
      close(in_fd);
      cleanup(destination);
      return 0;
    }
  }

  if (close(out_fd) < 0) {
    throw_errno(trace);
    close(in_fd);
    cleanup(destination);
    return 0;
  }

  close(in_fd);
  return source_offset;
}
