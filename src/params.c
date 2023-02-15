#include "params.h"
#include "messages.h"
#include "set.h"
#include "trace.h"
#include <stdlib.h>

struct params {
  const char *config_path;
  const char *privilege_dropping_path;
  struct set *ignored_write_mounts;
  struct set *ignored_exec_mounts;
};

struct params *parse_params(int argc, const char **argv, struct trace *trace) {
  struct params *params = TNULL(calloc(1, sizeof(struct params)), trace);
  struct set *ignored_write_mounts = TNULL(create_set(argc / 2, trace), trace);
  struct set *ignored_exec_mounts = TNULL(create_set(argc / 2, trace), trace);
  char opt = 0;

  for (int i = 1; i < argc && ok(trace); ++i) {
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
      params->config_path = argv[i];
      opt = 0;
      break;
    case 'd':
      params->privilege_dropping_path = argv[i];
      opt = 0;
      break;
    case 'e':
      add_to_set(argv[i], ignored_write_mounts, trace);
      opt = 0;
      break;
    case 'w':
      add_to_set(argv[i], ignored_exec_mounts, trace);
      opt = 0;
      break;
    case 'i':
      add_to_set(argv[i], ignored_write_mounts, trace);
      add_to_set(argv[i], ignored_exec_mounts, trace);
      opt = 0;
      break;
    default:
      throw_context(argv[i - 1], trace);
      throw_static(messages.params.unknown_option, trace);
    }
  }

  if (opt && ok(trace)) {
    throw_context(argv[argc - 1], trace);
    throw_static(messages.params.stray_option, trace);
  }

  if (!ok(trace)) {
    free_set(ignored_exec_mounts);
    free_set(ignored_write_mounts);
    free(params);
    return NULL;
  }

  params->ignored_exec_mounts = ignored_exec_mounts;
  params->ignored_write_mounts = ignored_write_mounts;
  return params;
}

const char *get_config_path(const struct params *params) {
  return params->config_path;
}

const char *get_privilege_dropping_path(const struct params *params) {
  return params->privilege_dropping_path;
}

const struct set *get_ignored_write_mounts(const struct params *params) {
  return params->ignored_write_mounts;
}

const struct set *get_ignored_exec_mounts(const struct params *params) {
  return params->ignored_exec_mounts;
}

void free_params(struct params *params) {
  if (params) {
    free_set(params->ignored_write_mounts);
    free_set(params->ignored_exec_mounts);
    free(params);
  }
}
