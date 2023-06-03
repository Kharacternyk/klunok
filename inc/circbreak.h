#include <time.h>

struct trace;

struct circuit_breaker *create_circuit_breaker(time_t seconds,
                                               struct trace *trace)
    __attribute__((warn_unused_result));
void arm(const struct circuit_breaker *circuit_breaker)
    __attribute__((nonnull));
void disarm(const struct circuit_breaker *circuit_breaker)
    __attribute__((nonnull));
void free_circuit_breaker(struct circuit_breaker *circuit_breaker);
