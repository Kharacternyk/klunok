#include "callback.h"
#include <stdlib.h>

struct callback {
  void (*function)(void *parameter);
  void *parameter;
  const struct callback *next;
};

struct callback *create_callback(void (*function)(void *), void *parameter,
                                 const struct callback *next) {
  struct callback *callback = malloc(sizeof(struct callback));
  if (!callback) {
    return NULL;
  }
  callback->function = function;
  callback->parameter = parameter;
  callback->next = next;
  return callback;
}

static void typed_simple_callback_function(bool *is_error) { *is_error = true; }
static void simple_callback_function(void *is_error) {
  typed_simple_callback_function(is_error);
}
struct callback *create_simple_callback(bool *is_error,
                                        const struct callback *next) {
  return create_callback(simple_callback_function, is_error, next);
}

void invoke_callback(const struct callback *callback) {
  while (callback && callback->function) {
    callback->function(callback->parameter);
    callback = callback->next;
  }
}
