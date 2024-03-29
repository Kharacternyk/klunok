#include <stdbool.h>
#include <stddef.h>

struct trace *create_trace() __attribute__((warn_unused_result));

bool ok(const struct trace *trace)
    __attribute__((pure, nonnull, warn_unused_result));
void try(struct trace *trace) __attribute__((nonnull));

void throw_static(const char *message, struct trace *trace)
    __attribute__((nonnull));
void throw_dynamic(const char *message, struct trace *trace)
    __attribute__((nonnull));
void throw_context(const char *message, struct trace *trace)
    __attribute__((nonnull));
void throw_errno(struct trace *trace) __attribute__((nonnull));

bool catch_static(const char *message, struct trace *trace)
    __attribute__((nonnull));

void finally(struct trace *trace) __attribute__((nonnull));
void finally_catch_all(struct trace *trace) __attribute__((nonnull));
void finally_rethrow_static(const char *message, struct trace *trace)
    __attribute__((nonnull));

void rethrow_context(const char *message, struct trace *trace)
    __attribute__((nonnull));

void unwind(int fd, const struct trace *trace) __attribute__((nonnull));

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
