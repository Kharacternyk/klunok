#include "circbreak.h"

int main() {
  struct trace *trace = create_trace();
  struct circuit_breaker *circuit_breaker = create_circuit_breaker(1, trace);
  arm(circuit_breaker);
  for (;;) {
    /*beep boop beep*/
  }
}
