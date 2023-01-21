#include <assert.h>
#include <string.h>

const char *get_file_extension(const char *path) {
  assert(path);
  const char *base_name = strrchr(path, '/');
  if (!base_name) {
    base_name = path;
  }
  const char *extension = strchr(base_name, '.');
  if (!extension) {
    extension = strrchr(path, 0);
  } else if (extension - base_name <= 1) {
    extension = strchr(extension + 1, '.');
    if (!extension) {
      extension = strrchr(path, 0);
    }
  }
  return extension;
}
