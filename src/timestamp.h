#include "callback.h"
#include <stddef.h>

char *get_timestamp(const char *format, size_t length_guess,
                    struct callback *error_callback);
