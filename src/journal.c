#include "journal.h"
#include "builder.h"
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
  struct builder *builder = create_builder(trace);
  if (*timestamp) {
    concat_string(timestamp, builder, trace);
    concat_char('\t', builder, trace);
  }
  if (*event) {
    concat_string(event, builder, trace);
    concat_char('\t', builder, trace);
  }
  if (pid) {
    concat_size(pid, builder, trace);
    concat_char('\t', builder, trace);
  }
  concat_string(path, builder, trace);
  concat_char('\n', builder, trace);

  size_t size_written = 0;
  while (ok(trace) && get_builder_length(builder) > size_written) {
    size_written += TNEG(
        write(journal->fd, build_string(builder), get_builder_length(builder)),
        trace);
  }

  free(timestamp);
  free_builder(builder);
}

void free_journal(struct journal *journal) {
  if (journal) {
    close(journal->fd);
    free(journal->timestamp_pattern);
    free(journal);
  }
}
