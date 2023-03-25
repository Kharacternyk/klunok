#include "params.h"
#include "buffer.h"
#include "set.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void test_params() {
  struct trace *trace = create_trace();
  const char *config_path = "config.lua";
  const char *drop_path = "/home/nazar";
  const char *ignored_path = "/mnt";
  const char *ignored_write_path = "/nix/store";
  const char *ignored_exec_path = "/tmp";
  const char *argv[] = {
      "klunok",     "-c", config_path,       "-d", drop_path,          "-i",
      ignored_path, "-w", ignored_exec_path, "-e", ignored_write_path,
  };
  int argc = sizeof argv / sizeof *argv;

  struct params *params = parse_params(argc, argv, trace);
  assert(ok(trace));
  assert(!strcmp(get_config_path(params), config_path));
  assert(!strcmp(get_privilege_dropping_path(params), drop_path));

  struct buffer_view *ignored_path_view =
      create_buffer_view(ignored_path, trace);
  struct buffer_view *ignored_write_path_view =
      create_buffer_view(ignored_write_path, trace);
  struct buffer_view *ignored_exec_path_view =
      create_buffer_view(ignored_exec_path, trace);
  assert(ok(trace));

  assert(is_in_set(ignored_path_view, get_ignored_write_mounts(params)));
  assert(is_in_set(ignored_path_view, get_ignored_exec_mounts(params)));
  assert(is_in_set(ignored_write_path_view, get_ignored_write_mounts(params)));
  assert(!is_in_set(ignored_write_path_view, get_ignored_exec_mounts(params)));
  assert(!is_in_set(ignored_exec_path_view, get_ignored_write_mounts(params)));
  assert(is_in_set(ignored_exec_path_view, get_ignored_exec_mounts(params)));

  free_buffer_view(ignored_path_view);
  free_buffer_view(ignored_write_path_view);
  free_buffer_view(ignored_exec_path_view);

  free_params(params);
  free(trace);
}
