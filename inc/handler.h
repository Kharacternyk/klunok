#include "config.h"
#include "trace.h"
#include <time.h>

struct handler;

struct handler *load_handler(const char *config_path, struct trace *trace);
void handle_open_exec(pid_t pid, int fd, struct handler *handler,
                      struct trace *trace);
void handle_close_write(pid_t pid, int fd, struct handler *handler,
                        struct trace *trace);
void handle_timeout(struct handler *handler, time_t *retry_after_seconds,
                    struct trace *trace);
void free_handler(struct handler *handler);
