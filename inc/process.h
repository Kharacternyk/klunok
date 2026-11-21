#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

struct trace;

bool same_process(int first_pidfd, int second_pidfd)
    __attribute__((warn_unused_result));
void require_pidfs(struct trace *trace) __attribute__((nonnull));

struct editors *create_editors(size_t size_guess, struct trace *trace)
    __attribute__((nonnull, warn_unused_result));
void track_editor(pid_t pid, int pidfd, struct editors *editors,
                  struct trace *trace);
bool is_editor(pid_t pid, int pidfd, struct editors *editors)
    __attribute__((nonnull, warn_unused_result));
void forget_editor(pid_t pid, struct editors *editors) __attribute__((nonnull));
void free_editors(struct editors *editors);
