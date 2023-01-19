#include <stdbool.h>
#include <stddef.h>

struct trace;

struct trace *create_trace();

const char *get_trace_message(const struct trace *trace);
void pop_trace_message(struct trace *trace);
size_t get_dropped_trace_message_count(const struct trace *trace);

void throw_static(const char *message, struct trace *trace);
void throw_dynamic(const char *message, struct trace *trace);
void throw_errno(struct trace *trace);

bool ok(const struct trace *trace);
bool catch_static(const char *message, struct trace *trace);
void catch_all(struct trace *trace);
