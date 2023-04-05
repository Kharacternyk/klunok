#include "circbreak.h"
#include "trace.h"
#include <assert.h>
#include <signal.h>
#include <stdlib.h>

struct circuit_breaker {
  time_t seconds;
  timer_t id;
};

struct circuit_breaker *create_circuit_breaker(time_t seconds,
                                               struct trace *trace) {
  struct sigevent sigevent = {
      .sigev_notify = SIGEV_SIGNAL,
      .sigev_signo = SIGALRM,
  };
  struct circuit_breaker *circuit_breaker =
      TNULL(malloc(sizeof(struct circuit_breaker)), trace);
  TNEG(timer_create(CLOCK_THREAD_CPUTIME_ID, &sigevent, &circuit_breaker->id),
       trace);
  if (ok(trace)) {
    circuit_breaker->seconds = seconds;
  }
  return circuit_breaker;
}

void arm(const struct circuit_breaker *circuit_breaker) {
  if (!circuit_breaker) {
    return;
  }
  struct itimerspec itimerspec = {
      .it_value.tv_sec = circuit_breaker->seconds,
  };
  assert(timer_settime(circuit_breaker->id, 0, &itimerspec, NULL) >= 0);
}

void disarm(const struct circuit_breaker *circuit_breaker) {
  if (!circuit_breaker) {
    return;
  }
  struct itimerspec itimerspec = {};
  assert(timer_settime(circuit_breaker->id, 0, &itimerspec, NULL) >= 0);
}

void free_circuit_breaker(struct circuit_breaker *circuit_breaker) {
  if (circuit_breaker) {
    assert(timer_delete(circuit_breaker->id) >= 0);
    free(circuit_breaker);
  }
}
