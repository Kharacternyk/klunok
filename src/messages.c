#include "messages.h"

/* clang-format off */
const struct translation messages = {
  .set = {
    .not_in_set = "Value is not in set",
  },
  .timestamp = {
    .overflow = "Version string exceeds the size limit",
  },
  .store = {
    .copy = {
      .file_does_not_exist = "File does not exist",
      .permission_denied = "Permission denied",
      .version_already_exists = "Version already exists",
    },
  },
  .config = {
    .is_static = "Configuration files are not supported",
  },
  .handler = {
    .version = {
      .has_slashes = "Version string contains slash characters",
    },
    .config = {
      .cannot_load = "Cannot load configuration file",
      .cannot_reload = "Cannot reload configuration file",
    },
    .linq = {
      .cannot_load = "Cannot load debouncing link queue",
      .cannot_reload = "Cannot reload debouncing link queue",
      .cannot_get_head = "Cannot get head of debouncing link queue",
      .cannot_push = "Cannot push file to debouncing link queue",
    },
    .store = {
      .cannot_copy = "Cannot copy file to store",
    },
  },
};
/* clang-format on */
