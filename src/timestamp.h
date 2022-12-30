#include "callback.h"
#include <stddef.h>

char *get_timestamp(const char *format, size_t max_length,
                    struct callback *error_callback);
