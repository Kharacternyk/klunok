#include <fcntl.h>

void create_parents(const char *path, mode_t mode, int *error_code);
void remove_empty_parents(const char *path, int *error_code);
