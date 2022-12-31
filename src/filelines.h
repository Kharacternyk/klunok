#include "set.h"

struct set *get_lines(const char *path, size_t average_line_length_guess,
                      struct callback *error_callback);
