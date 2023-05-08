#include "messages.h"
#include <assert.h>
#include <stddef.h>

void test_messages(void *unused) {
  char **messages_array = (char **)&messages;
  for (size_t i = 0; i < (sizeof messages / sizeof(char *)); ++i) {
    assert(messages_array[i]);
  }
}
