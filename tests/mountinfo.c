#include "mountinfo.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>

void test_mountinfo() {
  struct trace *trace = create_trace();
  struct mountinfo *mountinfo = create_mountinfo(trace);
  assert(ok(trace));
  int block_mount_count = 0;
  for (;;) {
    const char *mount = get_next_block_mount(mountinfo);
    if (!mount) {
      break;
    }
    ++block_mount_count;
    assert(*mount == '/');
  }
  /*FIXME can fail in the cloud*/
  assert(block_mount_count);
  free_mountinfo(mountinfo);
  free(trace);
}
