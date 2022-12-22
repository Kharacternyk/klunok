#include "../src/callback.h"
#include <assert.h>
#include <stdlib.h>

#define VALUE_OLD 9
#define VALUE_NEW 42

static void put_value(int *i) { *i = VALUE_NEW; }

static void put_value_void(void *p) { put_value(p); }

int main() {
  int i = VALUE_OLD;
  struct callback *callback = create_callback(put_value_void, &i);
  assert(i == VALUE_OLD);
  invoke_callback(callback);
  assert(i == VALUE_NEW);

  invoke_callback(create_callback(NULL, NULL));
  invoke_callback(NULL);
}
