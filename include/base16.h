#include <stdbool.h>
#include <stddef.h>

size_t decode_from_base16(const char *string, bool *is_malformed);
char *encode_to_base16(size_t value, int *error_code);
