#include "params.h"
#include "buffer.h"
#include "list.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void test_params() {
  struct trace *trace = create_trace();
  const char *config_path = "config.lua";
  const char *drop_path = "/home/nazar";
  const char *write_mount = "/home/nazar/src";
  const char *exec_mount = "/nix/store";
  const char *both_mount = "/home/nazar/.local/bin";
  const char *argv[] = {"klunok",    "-c", config_path, "-d", drop_path, "-w",
                        write_mount, "-e", exec_mount,  "-b", both_mount};
  int argc = sizeof argv / sizeof *argv;

  struct params *params = parse_params(argc, argv, trace);
  assert(ok(trace));
  assert(!strcmp(get_config_path(params), config_path));
  assert(!strcmp(get_privilege_dropping_path(params), drop_path));

  /*FIXME we shouldn't test the orer*/

  assert(!strcmp(get_value(peek(get_write_mounts(params))), both_mount));
  assert(!strcmp(get_value(get_next(peek(get_write_mounts(params)))),
                 write_mount));

  assert(!strcmp(get_value(peek(get_exec_mounts(params))), both_mount));
  assert(
      !strcmp(get_value(get_next(peek(get_exec_mounts(params)))), exec_mount));

  free_params(params);

  const char *empty_argv[] = {"klunok"};
  params = parse_params(1, empty_argv, trace);
  assert(ok(trace));
  assert(peek(get_write_mounts(params)));
  assert(peek(get_exec_mounts(params)));
  assert(get_privilege_dropping_path(params));
  free_params(params);

  free(trace);
}
