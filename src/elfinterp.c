#include "elfinterp.h"
#include "trace.h"
#include <elf.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __LP64__
#define ElfW(type) Elf64_##type
#else
#define ElfW(type) Elf32_##type
#endif

char *get_elf_interpreter(int exe_fd, struct trace *trace) {
  ElfW(Ehdr) elf_header;
  size_t total_read = 0;

  while (total_read < sizeof elf_header) {
    ssize_t iter_read =
        TNEG(read(exe_fd, &elf_header, sizeof elf_header), trace);
    if (!ok(trace) || !iter_read) {
      return NULL;
    }
    total_read += iter_read;
  }

  if (memcmp(elf_header.e_ident, ELFMAG, SELFMAG) || !elf_header.e_phoff) {
    return NULL;
  }

  if (lseek(exe_fd, elf_header.e_phoff, SEEK_SET) < 0) {
    return NULL;
  }

  for (size_t i = 0; i < elf_header.e_phnum; ++i) {
    ElfW(Phdr) program_header;
    size_t total_read = 0;
    while (total_read < sizeof program_header) {
      ssize_t iter_read =
          TNEG(read(exe_fd, &program_header, sizeof program_header), trace);
      if (!ok(trace) || !iter_read) {
        return NULL;
      }
      total_read += iter_read;
    }

    if (program_header.p_type == PT_INTERP) {
      if (lseek(exe_fd, program_header.p_offset, SEEK_SET) < 0) {
        return NULL;
      }
      char *result = malloc(program_header.p_filesz);
      if (!result) {
        return NULL;
      }

      size_t total_read = 0;
      while (total_read < program_header.p_filesz) {
        ssize_t iter_read =
            TNEG(read(exe_fd, result, program_header.p_filesz), trace);
        if (!ok(trace) || !iter_read) {
          return NULL;
        }
        total_read += iter_read;
      }

      char *resolved = TNULL(realpath(result, NULL), trace);
      free(result);
      return resolved;
    }
  }

  return NULL;
}
