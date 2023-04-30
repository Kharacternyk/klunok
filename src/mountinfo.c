#include "mountinfo.h"
#include "buffer.h"
#include "set.h"
#include "trace.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <unistd.h>

struct mountinfo {
  struct set *mounts;
};

static size_t count_lines(const char *string) {
  size_t count = 0;
  for (const char *cursor = string; *cursor; ++cursor) {
    if (*cursor == '\n') {
      ++count;
    }
  }
  return count;
}

static char *read_proc_file(const char *path, struct trace *trace) {
  long page_size = TNEG(sysconf(_SC_PAGE_SIZE), trace);
  int fd = TNEG(open(path, O_RDONLY), trace);
  char *page = TNULL(malloc(page_size + 1), trace);
  struct buffer *buffer = create_buffer(trace);

  ssize_t ssize;
  while ((ssize = TNEG(read(fd, page, page_size), trace)) > 0) {
    page[ssize] = 0;
    concat_string(page, buffer, trace);
  }

  free(page);
  if (fd >= 0) {
    close(fd);
  }
  return free_outer_buffer(buffer);
}

struct mountinfo *load_mountinfo(struct trace *trace) {
  struct mountinfo *mountinfo = TNULL(malloc(sizeof(struct mountinfo)), trace);
  char *proc_mounts_content = read_proc_file("/proc/self/mounts", trace);
  if (!ok(trace)) {
    free(mountinfo);
    return NULL;
  }

  char *cursor = proc_mounts_content;
  mountinfo->mounts = create_set(count_lines(proc_mounts_content), trace);

  for (char *record = strsep(&cursor, "\n"); ok(trace) && record;
       record = strsep(&cursor, "\n")) {
    strsep(&record, " ");
    if (record) {
      add(strsep(&record, " "), mountinfo->mounts, trace);
    }
  }

  free(proc_mounts_content);

  if (!ok(trace)) {
    free_mountinfo(mountinfo);
    return NULL;
  }

  return mountinfo;
}

char *make_mount(const char *path, const struct mountinfo *mountinfo,
                 struct trace *trace) {
  char *resolved_path = TNULL(realpath(path, NULL), trace);
  struct buffer_view *view = create_buffer_view(resolved_path, trace);
  if (!ok(trace)) {
    free(resolved_path);
    return NULL;
  }
  if (is_within(view, mountinfo->mounts)) {
    free_buffer_view(view);
    return resolved_path;
  }
  free_buffer_view(view);
  TNEG(mount(resolved_path, resolved_path, NULL, MS_BIND, NULL), trace);
  add(resolved_path, mountinfo->mounts, trace);
  return resolved_path;
}

void free_mountinfo(struct mountinfo *mountinfo) {
  if (mountinfo) {
    free_set(mountinfo->mounts);
    free(mountinfo);
  }
}
