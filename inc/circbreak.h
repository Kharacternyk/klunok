#include <time.h>

struct circuit_breaker;
struct trace;

struct circuit_breaker *create_circuit_breaker(time_t seconds,
                                               struct trace *trace);
void arm(const struct circuit_breaker *circuit_breaker);
void disarm(const struct circuit_breaker *circuit_breaker);
void free_circuit_breaker(struct circuit_breaker *circuit_breaker);
