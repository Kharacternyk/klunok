#include "params.h"
#include "constants.h"
#include "list.h"
#include "messages.h"
#include "trace.h"
#include <stdlib.h>
#include <unistd.h>

struct params {
  bool is_version_requested;
  bool is_help_requested;
  const char *config_path;
  const char *privilege_dropping_path;
  struct list *write_mounts;
  struct list *exec_mounts;
};

struct params *parse_params(int argc, const char **argv, struct trace *trace) {
  struct params *params = TNULL(calloc(1, sizeof(struct params)), trace);
  struct list *write_mounts = create_list(trace);
  struct list *exec_mounts = create_list(trace);
  char opt = 0;

  for (int i = 1; i < argc && ok(trace) && opt != 'h' && opt != 'v'; ++i) {
    switch (opt) {
    case 0:
      if (argv[i][0] != '-' || !argv[i][1] || argv[i][2]) {
        throw_context(argv[i], trace);
        throw_static(messages.params.unknown_option, trace);
      } else {
        opt = argv[i][1];
      }
      break;
    case 'c':
      if (params->config_path) {
        throw_context(argv[i - 1], trace);
        throw_static(messages.params.redefined_option, trace);
      } else {
        params->config_path = argv[i];
        opt = 0;
      }
      break;
    case 'd':
      if (params->config_path) {
        throw_context(argv[i - 1], trace);
        throw_static(messages.params.redefined_option, trace);
      } else {
        params->privilege_dropping_path = argv[i];
        opt = 0;
      }
      break;
    case 'w':
      join(argv[i], write_mounts, trace);
      opt = 0;
      break;
    case 'e':
      join(argv[i], exec_mounts, trace);
      opt = 0;
      break;
    default:
      throw_context(argv[i - 1], trace);
      throw_static(messages.params.unknown_option, trace);
    }
  }

  if (opt && ok(trace)) {
    switch (opt) {
    case 'h':
      params->is_help_requested = true;
      break;
    case 'v':
      params->is_version_requested = true;
      break;
    case 'c':
    case 'd':
    case 'w':
    case 'e':
      throw_context(argv[argc - 1], trace);
      throw_static(messages.params.stray_option, trace);
      break;
    default:
      throw_context(argv[argc - 1], trace);
      throw_static(messages.params.unknown_option, trace);
    }
  }

  if (ok(trace) && !peek(exec_mounts)) {
    join("/", exec_mounts, trace);
#ifdef WATCH_NIX_STORE
    join("/nix/store", exec_mounts, trace);
#endif
  }
  if (ok(trace) && !peek(write_mounts)) {
    join(".", write_mounts, trace);
  }
  if (ok(trace) && !params->privilege_dropping_path) {
    params->privilege_dropping_path = ".";
  }

  if (!ok(trace)) {
    free_list(exec_mounts);
    free_list(write_mounts);
    free(params);
    return NULL;
  }

  params->exec_mounts = exec_mounts;
  params->write_mounts = write_mounts;
  return params;
}

bool is_help_requested(const struct params *params) {
  return params->is_help_requested;
}

bool is_version_requested(const struct params *params) {
  return params->is_version_requested;
}

const char *get_config_path(const struct params *params) {
  return params->config_path;
}

const char *get_privilege_dropping_path(const struct params *params) {
  return params->privilege_dropping_path;
}

const struct list *get_write_mounts(const struct params *params) {
  return params->write_mounts;
}

const struct list *get_exec_mounts(const struct params *params) {
  return params->exec_mounts;
}

void free_params(struct params *params) {
  if (params) {
    free_list(params->write_mounts);
    free_list(params->exec_mounts);
    free(params);
  }
}
