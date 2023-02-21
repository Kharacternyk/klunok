#include "store.h"
#include "builder.h"
#include "messages.h"
#include "parents.h"
#include "trace.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

char *get_store_path(const char *filesystem_path, const char *version,
                     const char *store_root, struct trace *trace) {
  struct builder *builder = create_builder(trace);
  concat_string(store_root, builder, trace);
  /*FIXME needed only for simpler tests*/
  if (ok(trace) && *filesystem_path != '/') {
    concat_char('/', builder, trace);
  }
  concat_string(filesystem_path, builder, trace);
  concat_char('/', builder, trace);
  concat_string(version, builder, trace);
  return free_outer_builder(builder);
}

static void cleanup(char *store_path) {
  /*FIXME cleanup error reporting*/
  if (!store_path) {
    return;
  }
  struct trace *cleanup_trace = create_trace();
  if (cleanup_trace) {
    remove_empty_parents(store_path, cleanup_trace);
    catch_all(cleanup_trace);
    free(cleanup_trace);
  }
  free(store_path);
}

static void copy_to_store_with_offset(const char *filesystem_path,
                                      const char *version,
                                      const char *store_root, off_t *offset,
                                      struct trace *trace) {
  char *store_path =
      get_store_path(filesystem_path, version, store_root, trace);
  create_parents(store_path, trace);
  if (!ok(trace)) {
    return cleanup(store_path);
  }

  int in_fd = open(filesystem_path, O_RDONLY);
  if (in_fd < 0) {
    if (errno == ENOENT) {
      throw_static(messages.store.copy.file_does_not_exist, trace);
    } else if (errno == EACCES) {
      throw_static(messages.store.copy.permission_denied, trace);
    } else {
      throw_errno(trace);
    }
    return cleanup(store_path);
  }

  int out_fd = open(store_path, O_CREAT | O_WRONLY | O_EXCL, 0444);
  if (out_fd < 0) {
    if (errno == EEXIST) {
      throw_static(messages.store.copy.version_already_exists, trace);
    } else {
      throw_errno(trace);
    }
    close(in_fd);
    return cleanup(store_path);
  }

  struct stat in_fd_stat;
  if (fstat(in_fd, &in_fd_stat) < 0) {
    throw_errno(trace);
    close(out_fd);
    close(in_fd);
    return cleanup(store_path);
  }

  while (in_fd_stat.st_size) {
    size_t written_size = sendfile(out_fd, in_fd, offset, in_fd_stat.st_size);
    if (written_size > 0) {
      in_fd_stat.st_size -= written_size;
    } else if (written_size == 0) {
      break;
    } else {
      throw_errno(trace);
      close(out_fd);
      close(in_fd);
      return cleanup(store_path);
    }
  }

  if (close(out_fd) < 0) {
    throw_errno(trace);
    close(in_fd);
    return cleanup(store_path);
  }

  close(in_fd);
  free(store_path);
}

void copy_to_store(const char *filesystem_path, const char *version,
                   const char *store_root, struct trace *trace) {
  off_t offset = 0;
  copy_to_store_with_offset(filesystem_path, version, store_root, &offset,
                            trace);
}

void copy_delta_to_store(const char *filesystem_path, const char *version,
                         const char *cursor_name, const char *store_root,
                         struct trace *trace) {
  /* FIXME avoid one-char IO */
  char *cursor_path =
      get_store_path(filesystem_path, cursor_name, store_root, trace);
  create_parents(cursor_path, trace);
  int cursor_fd = TNEG(open(cursor_path, O_CREAT | O_RDWR, 0644), trace);
  off_t offset = 0;
  for (;;) {
    char digit = 0;
    if (TNEG(read(cursor_fd, &digit, 1), trace) <= 0 || digit < '0' ||
        digit > '9') {
      break;
    }
    offset *= 10;
    offset += digit - '0';
  }

  copy_to_store_with_offset(filesystem_path, version, store_root, &offset,
                            trace);
  TNEG(ftruncate(cursor_fd, 0), trace);
  TNEG(lseek(cursor_fd, 0, SEEK_SET), trace);

  struct builder *builder = create_builder(trace);
  concat_size(offset, builder, trace);

  for (size_t i = 0; ok(trace) && i < get_builder_length(builder); ++i) {
    TNEG(write(cursor_fd, get_string(builder) + i, 1), trace);
  }

  if (cursor_fd >= 0) {
    close(cursor_fd);
  }
  free_builder(builder);
  free(cursor_path);
}
