#include "mountinfo.h"
#include "buffer.h"
#include "trace.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct mountinfo {
  char *proc_mounts_content;
  char *cursor;
};

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

struct mountinfo *create_mountinfo(struct trace *trace) {
  struct mountinfo *mountinfo = TNULL(malloc(sizeof(struct mountinfo)), trace);
  char *proc_mounts_content = read_proc_file("/proc/self/mounts", trace);
  if (!ok(trace)) {
    free(mountinfo);
    free(proc_mounts_content);
    return NULL;
  }
  mountinfo->proc_mounts_content = proc_mounts_content;
  mountinfo->cursor = proc_mounts_content;
  return mountinfo;
}

const char *get_next_block_mount(struct mountinfo *mountinfo) {
  for (char *record = strsep(&mountinfo->cursor, "\n"); record;
       record = strsep(&mountinfo->cursor, "\n")) {
    if (*record == '/') {
      strsep(&record, " ");
      return strsep(&record, " ");
    }
  }
  return NULL;
}

void free_mountinfo(struct mountinfo *mountinfo) {
  if (mountinfo) {
    free(mountinfo->proc_mounts_content);
    free(mountinfo);
  }
}
