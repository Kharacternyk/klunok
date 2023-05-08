#include "mountinfo.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void test_mountinfo(struct trace *trace) {
  struct mountinfo *mountinfo = load_mountinfo(trace);
  assert(ok(trace));

  char *mount = make_mount("/", mountinfo, trace);
  assert(ok(trace));
  assert(!strcmp(mount, "/"));

  free_mountinfo(mountinfo);
  free(mount);
}
