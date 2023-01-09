#include "base16.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

size_t decode_from_base16(const char *string, bool *is_malformed) {
  size_t result;
  if (sscanf(string, "%zx", &result) < 1) {
    *is_malformed = true;
    return 0;
  }
  return result;
}

char *encode_to_base16(size_t value, int *error_code) {
  /*FIXME assumes 8 bits in a character*/
  int zero_count = sizeof(size_t) * 2;
  size_t length = snprintf(NULL, 0, "%0*zx", zero_count, value);
  size_t size = length + 1;
  char *result = malloc(size);
  if (!result) {
    *error_code = errno;
    return NULL;
  }

  int code = snprintf(result, size, "%0*zx", zero_count, value);
  if (code < 0) {
    /*FIXME how to get error from snprintf?*/
    *error_code = -code;
    return NULL;
  }

  return result;
}
