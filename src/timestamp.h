#include "callback.h"
#include <stddef.h>

char *get_timestamp(const char *format, size_t max_length,
                    const struct callback *error_callback, bool *is_overflow);
