#include "config.h"
#include <errno.h>
#include <stdlib.h>

static const char *const store_root = "./klunok/store";
static const char *const queue_path = "./klunok/queue";
static const char *const version_pattern = "v%Y-%m-%d-%H-%M";
static const size_t debounce_seconds = 60;
static const size_t version_max_length = 80;
static const size_t path_length_guess = 1024;
static const pid_t max_pid_guess = 1 << 16;
static const char *const editors[] = {
    "vi",
    "vim",
    "nvim",
    "rstudio",
};

struct config {
  struct set *editors;
};

struct config *load_config(const char *path, int *error_code,
                           char **error_message) {
  struct config *config = malloc(sizeof(struct config));
  if (!config) {
    *error_code = errno;
    return NULL;
  }

  size_t editors_length = sizeof editors / sizeof editors[0];
  config->editors = create_set(editors_length, error_code);
  if (*error_code) {
    goto config_cleanup;
  }

  for (size_t i = 0; i < editors_length; ++i) {
    add_to_set(editors[i], config->editors, error_code);
    if (*error_code) {
      goto editors_cleanup;
    }
  }

  return config;

editors_cleanup:
  free_set(config->editors);
config_cleanup:
  free(config);
  return NULL;
}

const struct set *get_configured_editors(const struct config *config) {
  return config->editors;
}

const char *get_configured_store_root(const struct config *config) {
  return store_root;
}

const char *get_configured_queue_path(const struct config *config) {
  return queue_path;
}

const char *get_configured_version_pattern(const struct config *config) {
  return version_pattern;
}

size_t get_configured_debounce_seconds(const struct config *config) {
  return debounce_seconds;
}

size_t get_configured_version_max_length(const struct config *config) {
  return version_max_length;
}

size_t get_configured_path_length_guess(const struct config *config) {
  return path_length_guess;
}

pid_t get_configured_max_pid_guess(const struct config *config) {
  return max_pid_guess;
}

void free_config(struct config *config) {
  if (config) {
    free_set(config->editors);
    free(config);
  }
}
