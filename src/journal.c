#include "journal.h"
#include "buffer.h"
#include "parents.h"
#include "timestamp.h"
#include "trace.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct journal {
  int fd;
  char *timestamp_pattern;
};

struct journal *open_journal(const char *path, const char *timestamp_pattern,
                             struct trace *trace) {
  if (!path) {
    return NULL;
  }
  create_parents(path, trace);
  struct journal *journal = TNULL(malloc(sizeof(struct journal)), trace);
  if (!ok(trace)) {
    return NULL;
  }
  journal->fd = TNEG(open(path, O_CREAT | O_WRONLY | O_APPEND, 0644), trace);
  journal->timestamp_pattern = TNULL(strdup(timestamp_pattern), trace);
  if (!ok(trace)) {
    if (journal->fd > 0) {
      close(journal->fd);
    }
    free(journal->timestamp_pattern);
    free(journal);
    return NULL;
  }
  return journal;
}

void write_to_journal(const char *event, pid_t pid, const char *path,
                      const struct journal *journal, struct trace *trace) {
  if (!journal || !event || !ok(trace)) {
    return;
  }
  char *timestamp =
      get_timestamp(journal->timestamp_pattern, /*FIXME*/ NAME_MAX, trace);
  struct buffer *buffer = create_buffer(trace);
  if (!ok(trace)) {
    free(timestamp);
    free_buffer(buffer);
    return;
  }
  if (*timestamp) {
    concat_string(timestamp, buffer, trace);
    concat_char('\t', buffer, trace);
  }
  if (*event) {
    concat_string(event, buffer, trace);
    concat_char('\t', buffer, trace);
  }
  if (pid) {
    concat_size(pid, buffer, trace);
    concat_char('\t', buffer, trace);
  }
  concat_string(path, buffer, trace);
  concat_char('\n', buffer, trace);

  size_t size_written = 0;
  while (ok(trace) && get_length(get_view(buffer)) > size_written) {
    size_written += TNEG(write(journal->fd, get_string(get_view(buffer)),
                               get_length(get_view(buffer))),
                         trace);
  }

  free(timestamp);
  free_buffer(buffer);
}

void free_journal(struct journal *journal) {
  if (journal) {
    close(journal->fd);
    free(journal->timestamp_pattern);
    free(journal);
  }
}
