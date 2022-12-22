#include "callback.h"
#include <stdlib.h>

struct callback {
  void (*function)(void *parameter);
  void *parameter;
};

struct callback *create_callback(void (*function)(void *), void *parameter) {
  struct callback *callback = malloc(sizeof(struct callback));
  callback->function = function;
  callback->parameter = parameter;
  return callback;
}

void invoke_callback(struct callback *callback) {
  if (callback && callback->function) {
    callback->function(callback->parameter);
  }
}
