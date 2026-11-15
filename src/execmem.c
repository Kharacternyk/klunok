#include "execmem.h"
#include "trace.h"
#include <errno.h>
#include <seccomp.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/personality.h>

void refuse_executable_mappings(struct trace *trace) {
  unsigned long current_personality = TNEG(personality(UINT32_MAX), trace);
  if (ok(trace) && current_personality & READ_IMPLIES_EXEC) {
    TNEG(personality(current_personality & ~READ_IMPLIES_EXEC), trace);
  }

  scmp_filter_ctx filter = TNULL(seccomp_init(SCMP_ACT_ALLOW), trace);
  if (!ok(trace)) {
    return;
  }

  const uint32_t deny = SCMP_ACT_ERRNO(EPERM);
  TRET(seccomp_attr_set(filter, SCMP_FLTATR_ACT_BADARCH, deny), trace);
  TRET(seccomp_rule_add(filter, deny, SCMP_SYS(personality), 0), trace);

  uint32_t arch = seccomp_arch_native();
  const int mapping_syscalls[] = {
      SCMP_SYS(mmap),
      SCMP_SYS(mmap2),
      SCMP_SYS(mprotect),
      SCMP_SYS(pkey_mprotect),
  };

  for (size_t i = 0;
       ok(trace) && i < sizeof mapping_syscalls / sizeof *mapping_syscalls;
       ++i) {
    int syscall = mapping_syscalls[i];

    if (syscall >= 0) {
      if (syscall == SCMP_SYS(mmap) &&
          (arch == SCMP_ARCH_X86 || arch == SCMP_ARCH_ARM)) {
        TRET(seccomp_rule_add(filter, deny, syscall, 0), trace);
      } else {
        TRET(
            seccomp_rule_add(filter, deny, syscall, 1,
                             SCMP_A2(SCMP_CMP_MASKED_EQ, PROT_EXEC, PROT_EXEC)),
            trace);
      }
    }
  }

  TRET(seccomp_load(filter), trace);
  seccomp_release(filter);
}
