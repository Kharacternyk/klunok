#include <time.h>

struct trace;

struct handler *load_handler(const char *config_path,
                             size_t common_parent_path_length,
                             struct trace *trace);
void handle_open_exec(pid_t pid, int fd, struct handler *handler,
                      struct trace *trace);
void handle_close_write(pid_t pid, int fd, struct handler *handler,
                        struct trace *trace);
time_t handle_timeout(struct handler *handler, struct trace *trace);
void free_handler(struct handler *handler);
