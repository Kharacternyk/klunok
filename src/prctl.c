#include "prctl.h"
#include "buffer.h"
#include "messages.h"
#include "trace.h"
#include <sys/prctl.h>

/* Keep libc's declarations separate from linux/prctl.h for musl. */
void set_process_option(int option, unsigned long value, struct trace *trace) {
  if (!ok(trace)) {
    return;
  }

  TNEG(prctl(option, value, 0L, 0L, 0L), trace);

  if (!ok(trace)) {
    struct trace *format_trace = create_trace();

    if (format_trace) {
      struct buffer *number = create_buffer(format_trace);
      size_t magnitude = option;

      if (option < 0) {
        concat_char('-', number, format_trace);
        magnitude = -magnitude;
      }

      concat_size(magnitude, number, format_trace);

      if (ok(format_trace)) {
        throw_context(get_string(get_view(number)), trace);
      }

      free_buffer(number);
      free_trace(format_trace);
    }

    throw_static(messages.prctl.cannot_set_option, trace);
  }
}
