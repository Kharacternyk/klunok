#include "journal.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>

void test_journal(struct trace *trace) {
  const char *journal_path = "./var/journal";
  const char *timestamp_pattern = "1";
  struct journal *journal =
      open_journal(journal_path, timestamp_pattern, trace);
  assert(ok(trace));
  note(NULL, 0, "", journal, trace);
  assert(ok(trace));
  note("3", 5, "7", journal, trace);
  assert(ok(trace));

  struct stat journal_stat;
  assert(stat(journal_path, &journal_stat) >= 0);
  assert(journal_stat.st_size == 8);

  free_journal(journal);
  journal = open_journal(NULL, "", trace);
  assert(ok(trace));
  note("blah", 0, "/", journal, trace);
  assert(ok(trace));
}
