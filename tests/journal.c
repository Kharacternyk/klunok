#include "journal.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>

void test_journal() {
  const char *journal_path = "./var/journal";
  const char *timestamp_pattern = "1";
  struct trace *trace = create_trace();
  struct journal *journal =
      open_journal(journal_path, timestamp_pattern, trace);
  assert(ok(trace));
  write_to_journal(NULL, 0, "", journal, trace);
  assert(ok(trace));
  write_to_journal("3", 5, "7", journal, trace);
  assert(ok(trace));

  struct stat journal_stat;
  assert(stat(journal_path, &journal_stat) >= 0);
  assert(journal_stat.st_size == 8);

  free_journal(journal);
  journal = open_journal(NULL, "", trace);
  assert(ok(trace));
  write_to_journal("blah", 0, "/", journal, trace);
  assert(ok(trace));

  free(trace);
}
