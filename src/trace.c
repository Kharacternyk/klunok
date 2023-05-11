#include "trace.h"
#include "logstep.h"
#include "messages.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

struct frame {
  union {
    char *dynamic_message;
    const char *static_message;
  };
  bool is_dynamic;
  bool is_context;
  struct frame *next;
};

struct trace {
  size_t dropped_frame_count;
  struct frame *head;
  size_t pre_throw_depth;
  size_t post_throw_depth;
};

struct trace *create_trace() {
  return calloc(1, sizeof(struct trace));
}

static void pop_trace_message(struct trace *trace) {
  struct frame *new_head = trace->head->next;
  if (trace->head->is_dynamic) {
    free(trace->head->dynamic_message);
  }
  free(trace->head);
  trace->head = new_head;
}

static void throw_common(const char *message, bool is_dynamic, bool is_context,
                         struct trace *trace) {
  assert(message);
  assert(trace);
  assert(is_dynamic || !is_context);
  struct frame *frame = malloc(sizeof(struct frame));
  if (!frame) {
    ++trace->dropped_frame_count;
    return;
  }
  if (is_dynamic) {
    frame->dynamic_message = strdup(message);
    if (!frame->dynamic_message) {
      ++trace->dropped_frame_count;
      return free(frame);
    }
  } else {
    frame->static_message = message;
  }
  frame->is_dynamic = is_dynamic;
  frame->is_context = is_context;
  frame->next = trace->head;
  trace->head = frame;
}

void throw_static(const char *message, struct trace *trace) {
  throw_common(message, false, false, trace);
}

void throw_dynamic(const char *message, struct trace *trace) {
  throw_common(message, true, false, trace);
}

void throw_context(const char *message, struct trace *trace) {
  throw_common(message, true, true, trace);
}

void throw_errno(struct trace *trace) { throw_dynamic(strerror(errno), trace); }

bool ok(const struct trace *trace) {
  assert(trace);
  return !trace->head && !trace->dropped_frame_count;
}

void try(struct trace *trace) {
  if (ok(trace)) {
    ++trace->pre_throw_depth;
  } else {
    ++trace->post_throw_depth;
  }
}

bool catch_static(const char *message, struct trace *trace) {
  assert(message);
  assert(trace);
  if (trace->dropped_frame_count || trace->post_throw_depth) {
    return false;
  }
  if (trace->head && !trace->head->is_dynamic &&
      trace->head->static_message == message) {
    while (trace->head) {
      pop_trace_message(trace);
    }
    return true;
  }
  return false;
}

static bool decrement_depth(struct trace *trace) {
  if (trace->post_throw_depth) {
    --trace->post_throw_depth;
    return false;
  } else {
    assert(trace->pre_throw_depth);
    --trace->pre_throw_depth;
    return true;
  }
}

void finally_catch_all(struct trace *trace) {
  assert(trace);
  trace->dropped_frame_count = 0;
  while (trace->head) {
    pop_trace_message(trace);
  }
  decrement_depth(trace);
}

void finally(struct trace *trace) { decrement_depth(trace); }

void finally_rethrow_static(const char *message, struct trace *trace) {
  if (decrement_depth(trace) && !ok(trace)) {
    throw_static(message, trace);
  }
}

void rethrow_context(const char *message, struct trace *trace) {
  assert(trace->post_throw_depth || trace->pre_throw_depth);
  if (!trace->post_throw_depth && !ok(trace)) {
    throw_context(message, trace);
  }
}

void unwind(int fd, const struct trace *trace) {
  size_t depth = 0;

  for (struct frame *frame = trace->head; frame; frame = frame->next) {
    const char *prefix = NULL;

    if (depth) {
      if (frame->is_context) {
        prefix = messages.trace.which_is;
      } else {
        prefix = messages.trace.because_of;
      }
    }

    logstep(fd, prefix, frame->static_message, depth);
    ++depth;
  }

  for (size_t i = 0; i < trace->dropped_frame_count; ++i) {
    logstep(fd, messages.trace.message_dropped, NULL, depth);
    ++depth;
  }
}
