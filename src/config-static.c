#include "config.h"
#include <errno.h>
#include <stdlib.h>

const char *const store = "./klunok/store";
const char *const queue = "./klunok/queue";
const char *const version_pattern = "v%Y-%m-%d-%H-%M";
const size_t version_max_length = 80;
const size_t path_length_guess = 1024;
const pid_t max_pid_guess = 1 << 16;
const char *const editors[] = {
    "vi",
    "vim",
    "nvim",
    "rstudio",
};

struct config {
  struct store *store;
  struct linq *queue;
  struct set *editors;
};

struct config *load_config(const char *path, int *error_code,
                           char **error_message) {
  struct config *config = malloc(sizeof(struct config));
  if (!config) {
    *error_code = errno;
    return NULL;
  }

  config->store = create_store(store, error_code);
  if (*error_code) {
    goto config_cleanup;
  }

  config->queue = load_linq(queue, error_code);
  if (*error_code) {
    goto store_cleanup;
  }

  size_t editors_length = sizeof editors / sizeof editors[0];
  config->editors = create_set(editors_length, error_code);
  if (*error_code) {
    goto queue_cleanup;
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
queue_cleanup:
  free_linq(config->queue);
store_cleanup:
  free_store(config->store);
config_cleanup:
  free(config);
  return NULL;
}

const struct store *get_configured_store(const struct config *config) {
  return config->store;
}

const struct set *get_configured_editors(const struct config *config) {
  return config->editors;
}

const char *get_configured_version_pattern(const struct config *config) {
  return version_pattern;
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
    free_store(config->store);
    free_linq(config->queue);
    free_set(config->editors);
    free(config);
  }
}
