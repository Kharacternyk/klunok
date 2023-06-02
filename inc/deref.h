#include <stddef.h>

struct trace;

char *deref_fd(int fd, size_t length_guess, struct trace *trace)
    __attribute__((warn_unused_result));
