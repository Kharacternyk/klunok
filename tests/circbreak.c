#include "circbreak.h"
#include "trace.h"

void test_circbreak(struct trace *trace) {
  struct circuit_breaker *circuit_breaker = create_circuit_breaker(1, trace);
  arm(circuit_breaker);
  for (;;) {
    /*beep boop beep*/
  }
}
