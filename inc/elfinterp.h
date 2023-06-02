struct trace;

char *get_elf_interpreter(int exe_fd, struct trace *trace)
    __attribute__((warn_unused_result));
