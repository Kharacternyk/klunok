#include <fcntl.h>

struct trace;

struct journal *open_journal(const char *path, const char *timestamp_pattern,
                             struct trace *trace)
    __attribute__((warn_unused_result));
void note(const char *event, pid_t pid, const char *path,
          const struct journal *journal, struct trace *trace);
void free_journal(struct journal *journal);
