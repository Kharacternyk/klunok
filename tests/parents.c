#include "parents.h"
#include "trace.h"
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#define DIRECTORY "parents/abc/defgh/123"

void test_parents(struct trace *trace) {
  assert(access(DIRECTORY, F_OK) != 0);
  create_parents(DIRECTORY "/file", trace);
  assert(ok(trace));
  assert(access(DIRECTORY, F_OK) == 0);
  remove_empty_parents(DIRECTORY "/file", trace);
  assert(ok(trace));
  assert(access(DIRECTORY, F_OK) != 0);

  assert(get_common_parent_path_length("/abc/def", "/") == 1);
  assert(get_common_parent_path_length("/abc/def", "/ab") == 1);
  assert(get_common_parent_path_length("/abc/def", "/abc/de") == 5);
  assert(get_common_parent_path_length("/abc/def", "/abc/defgh") == 5);
  assert(get_common_parent_path_length("/abc/def", "/abc/def") == 9);
  assert(get_common_parent_path_length("/abc/def", "/abc/def/ghi") == 9);
  assert(get_common_parent_path_length("/abc/def", "/ayc/def") == 1);

  assert(get_common_parent_path_length("/", "/abc/def") == 1);
  assert(get_common_parent_path_length("/ab", "/abc/def") == 1);
  assert(get_common_parent_path_length("/abc/de", "/abc/def") == 5);
  assert(get_common_parent_path_length("/abc/defgh", "/abc/def") == 5);
  assert(get_common_parent_path_length("/abc/def", "/abc/def") == 9);
  assert(get_common_parent_path_length("/abc/def/ghi", "/abc/def") == 9);
  assert(get_common_parent_path_length("/ayc/def", "/abc/def") == 1);
}
