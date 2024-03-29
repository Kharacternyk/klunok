#include "sync.h"
#include "buffer.h"
#include "messages.h"
#include "parents.h"
#include "trace.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

static void clean_up(const char *destination) {
  /*FIXME cleanup error reporting*/
  struct trace *cleanup_trace = create_trace();
  if (cleanup_trace) {
    try(cleanup_trace);
    remove_empty_parents(destination, cleanup_trace);
    finally_catch_all(cleanup_trace);
    free(cleanup_trace);
  }
}

off_t sync_file(const char *destination, const char *source,
                off_t source_offset, struct trace *trace) {
  if (!ok(trace)) {
    return 0;
  }

  create_parents(destination, trace);
  if (!ok(trace)) {
    clean_up(destination);
    return 0;
  }

  int in_fd = open(source, O_RDONLY);
  if (in_fd < 0) {
    if (errno == ENOENT) {
      throw_static(messages.sync.source_does_not_exist, trace);
    } else if (errno == EACCES) {
      throw_static(messages.sync.source_permission_denied, trace);
    } else {
      throw_errno(trace);
    }
    clean_up(destination);
    return 0;
  }

  int out_fd = open(destination, O_CREAT | O_WRONLY | O_EXCL, 0444);
  if (out_fd < 0) {
    if (errno == EEXIST) {
      throw_static(messages.sync.destination_already_exists, trace);
    } else {
      throw_errno(trace);
    }
    close(in_fd);
    clean_up(destination);
    return 0;
  }

  struct stat in_fd_stat;
  if (fstat(in_fd, &in_fd_stat) < 0) {
    throw_errno(trace);
    close(out_fd);
    close(in_fd);
    clean_up(destination);
    return 0;
  }

  if (!S_ISREG(in_fd_stat.st_mode)) {
    throw_static(messages.sync.source_is_not_regular_file, trace);
    close(out_fd);
    close(in_fd);
    clean_up(destination);
    return 0;
  }

  while (in_fd_stat.st_size) {
    ssize_t written_size =
        sendfile(out_fd, in_fd, &source_offset, in_fd_stat.st_size);
    if (written_size > 0) {
      in_fd_stat.st_size -= written_size;
    } else if (written_size == 0) {
      break;
    } else {
      throw_errno(trace);
      close(out_fd);
      close(in_fd);
      clean_up(destination);
      return 0;
    }
  }

  if (close(out_fd) < 0) {
    throw_errno(trace);
    close(in_fd);
    clean_up(destination);
    return 0;
  }

  close(in_fd);
  return source_offset;
}

void sync_shallow_tree(const char *destination, const char *source,
                       const char *existence_filter_root, struct trace *trace) {
  create_parents(destination, trace);

  if (ok(trace) && mkdir(destination, 0755)) {
    if (errno == EEXIST) {
      throw_static(messages.sync.destination_already_exists, trace);
    } else {
      throw_errno(trace);
    }
  }

  int destination_fd = TNEG(open(destination, O_RDONLY), trace);
  char *paths[] = {TNULL(strdup(source), trace), NULL};

  if (!ok(trace)) {
    if (destination_fd > 0) {
      close(destination_fd);
    }
    free(*paths);
    clean_up(destination);
    return;
  }

  FTS *fts = fts_open(paths, FTS_PHYSICAL | FTS_NOSTAT | FTS_NOCHDIR, NULL);

  if (!fts) {
    if (errno == ENOENT) {
      throw_static(messages.sync.source_does_not_exist, trace);
    } else if (errno == EACCES) {
      throw_static(messages.sync.source_permission_denied, trace);
    } else {
      throw_errno(trace);
    }
  }

  assert(!strstr(destination, "//"));
  size_t source_length = strlen(source);

  struct buffer *filter_path = create_buffer(trace);
  concat_string(existence_filter_root, filter_path, trace);
  concat_char('/', filter_path, trace);
  size_t filter_root_length = get_length(get_view(filter_path));

  for (FTSENT *entry = fts_read(fts); entry && ok(trace);
       entry = fts_read(fts)) {
    const char *relative_path = entry->fts_path + source_length;

    if (!*relative_path) {
      continue;
    }
    if (*relative_path == '/') {
      ++relative_path;
    }

    set_length(filter_root_length, filter_path);
    concat_string(relative_path, filter_path, trace);
    if (!ok(trace)) {
      break;
    }

    switch (entry->fts_info) {
    case FTS_D:
      if (!access(get_string(get_view(filter_path)), F_OK)) {
        TNEG(mkdirat(destination_fd, relative_path, 0755), trace);
      }
      break;
    case FTS_DP:
      if (access(get_string(get_view(filter_path)), F_OK)) {
        TNEG(rmdir(entry->fts_path), trace);
      }
      break;
    case FTS_NSOK:
      if (!access(get_string(get_view(filter_path)), F_OK)) {
        TNEG(
            linkat(AT_FDCWD, entry->fts_path, destination_fd, relative_path, 0),
            trace);
      } else {
        TNEG(unlink(entry->fts_path), trace);
      }
      break;
    case FTS_DNR:
    case FTS_ERR:
      throw_dynamic(strerror(entry->fts_errno), trace);
      break;
    }
  }

  if (ok(trace) && errno) {
    throw_errno(trace);
  }

  if (fts) {
    fts_close(fts);
  }

  close(destination_fd);
  free(*paths);
  free_buffer(filter_path);

  if (!ok(trace)) {
    clean_up(destination);
  }
}
