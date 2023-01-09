#include <stdbool.h>
#include <stddef.h>

char *get_timestamp(const char *format, size_t max_length, int *error_code,
                    bool *is_overflow);
