#include "trace.h"
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
  struct frame *next;
};

struct trace {
  size_t dropped_frame_count;
  struct frame *head;
};

struct trace *create_trace() {
  return calloc(1, sizeof(struct trace));
}

const char *get_trace_message(const struct trace *trace) {
  assert(trace);
  if (trace->head) {
    return trace->head->static_message;
  }
  return NULL;
}

void pop_trace_message(struct trace *trace) {
  assert(trace);
  assert(trace->head);
  struct frame *new_head = trace->head->next;
  if (trace->head->is_dynamic) {
    free(trace->head->dynamic_message);
  }
  free(trace->head);
  trace->head = new_head;
}

void trace_static(const char *message, struct trace *trace) {
  assert(message);
  assert(trace);
  struct frame *frame = malloc(sizeof(struct frame));
  if (!frame) {
    ++trace->dropped_frame_count;
    return;
  }
  frame->static_message = message;
  frame->is_dynamic = false;
  frame->next = trace->head;
  trace->head = frame;
}

void trace_dynamic(const char *message, struct trace *trace) {
  assert(message);
  assert(trace);
  struct frame *frame = malloc(sizeof(struct frame));
  if (!frame) {
    ++trace->dropped_frame_count;
    return;
  }
  frame->dynamic_message = strdup(message);
  if (!frame->dynamic_message) {
    ++trace->dropped_frame_count;
    free(frame);
    return;
  }
  frame->is_dynamic = true;
  frame->next = trace->head;
  trace->head = frame;
}

void trace_errno(struct trace *trace) { trace_dynamic(strerror(errno), trace); }

size_t get_dropped_trace_message_count(const struct trace *trace) {
  assert(trace);
  return trace->dropped_frame_count;
}

bool ok(const struct trace *trace) {
  assert(trace);
  return !trace->head && !trace->dropped_frame_count;
}

void clear(struct trace *trace) {
  assert(trace);
  trace->dropped_frame_count = 0;
  while (trace->head) {
    pop_trace_message(trace);
  }
}
