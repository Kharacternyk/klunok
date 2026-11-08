#include "prctl.h"
#include "messages.h"
#include "trace.h"
#include <assert.h>
#include <fcntl.h>
#include <linux/prctl.h>
#include <string.h>
#include <unistd.h>

void test_prctl(struct trace *trace) {
  set_process_option(PR_SET_NAME, (unsigned long)"klunok-prctl", trace);
  assert(ok(trace));

  try(trace);
  set_process_option(-1, 0, trace);
  set_process_option(PR_SET_NAME, (unsigned long)"unexpected", trace);
  assert(catch_static(messages.prctl.cannot_set_option, trace));
  finally(trace);

  int fd = open("/proc/self/comm", O_RDONLY);
  assert(fd >= 0);
  char name[32] = {0};
  assert(read(fd, name, sizeof(name) - 1) > 0);
  assert(!strcmp(name, "klunok-prctl\n"));
  assert(!close(fd));
}
