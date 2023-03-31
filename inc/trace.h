#include <stdbool.h>
#include <stddef.h>

struct trace *create_trace();

const char *get_trace_message(const struct trace *trace);
bool is_trace_message_context(const struct trace *trace);
void pop_trace_message(struct trace *trace);
size_t get_dropped_trace_message_count(const struct trace *trace);

void throw_static(const char *message, struct trace *trace);
void throw_dynamic(const char *message, struct trace *trace);
void throw_context(const char *message, struct trace *trace);
void throw_errno(struct trace *trace);

bool ok(const struct trace *trace);
bool catch_static(const char *message, struct trace *trace);
void catch_all(struct trace *trace);

void rethrow_check(struct trace *trace);
void rethrow_static(const char *message, struct trace *trace);
void rethrow_context(const char *message, struct trace *trace);

#define TNEG(call, trace)                                                      \
  ({                                                                           \
    typeof(call) _tneg_result = -1;                                            \
    if (ok(trace)) {                                                           \
      _tneg_result = call;                                                     \
      if (_tneg_result < 0) {                                                  \
        throw_errno(trace);                                                    \
      }                                                                        \
    }                                                                          \
    _tneg_result;                                                              \
  })

#define TNULL(call, trace)                                                     \
  ({                                                                           \
    typeof(call) _tnull_result = NULL;                                         \
    if (ok(trace)) {                                                           \
      _tnull_result = call;                                                    \
      if (!_tnull_result) {                                                    \
        throw_errno(trace);                                                    \
      }                                                                        \
    }                                                                          \
    _tnull_result;                                                             \
  })
