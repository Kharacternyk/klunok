#include "config.h"
#include <time.h>

struct handler;

struct handler *load_handler(const char *config_path, int *error_code,
                             char **error_message);
void handle_open_exec(pid_t pid, int fd, struct handler *handler,
                      int *error_code);
void handle_close_write(pid_t pid, int fd, struct handler *handler,
                        bool *is_config_changed, int *error_code,
                        char **error_message);
void handle_timeout(struct handler *handler, time_t *retry_after_seconds,
                    bool *is_version_invalid, int *error_code,
                    int *cleanup_error_code);
