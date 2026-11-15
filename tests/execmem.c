#include "execmem.h"
#include "trace.h"
#include <assert.h>
#include <errno.h>
#include <iconv.h>
#include <seccomp.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <unistd.h>

void test_execmem(struct trace *trace) {
  iconv_t converter = iconv_open("ISO-8859-2", "UTF-8");
  assert(converter != (iconv_t)-1);
  assert(!iconv_close(converter));

  refuse_executable_mappings(trace);
  assert(ok(trace));

  assert(iconv_open("ISO-2022-JP", "UTF-8") == (iconv_t)-1);
  assert(errno == EINVAL);

  long page_size = sysconf(_SC_PAGESIZE);
  assert(page_size > 0);

  char *data = mmap(NULL, page_size, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  assert(data != MAP_FAILED);

  *data = 42;

  assert(!mprotect(data, page_size, PROT_READ));
  assert(*data == 42);
  assert(mprotect(data, page_size, PROT_READ | PROT_EXEC) == -1);
  assert(errno == EPERM);
  assert(!munmap(data, page_size));

  assert(mmap(NULL, page_size, PROT_READ | PROT_EXEC,
              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0) == MAP_FAILED);
  assert(errno == EPERM);
}
