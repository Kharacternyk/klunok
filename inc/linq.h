#include <time.h>

struct trace;

struct linq *load_linq(const char *path, time_t debounce_seconds,
                       size_t entry_count_guess, size_t entry_length_guess,
                       struct trace *trace) __attribute__((warn_unused_result));
void push(const char *path, struct linq *linq, struct trace *trace);
void pop_head(struct linq *linq, struct trace *trace);
void redebounce(time_t debounce_seconds, struct linq *linq);
void free_linq(struct linq *linq);

struct linq_head *get_head(struct linq *linq, struct trace *trace)
    __attribute__((warn_unused_result));
const char *get_path(const struct linq_head *head)
    __attribute__((pure, warn_unused_result));
time_t get_pause(const struct linq_head *head)
    __attribute__((pure, warn_unused_result));
void free_linq_head(struct linq_head *head);
