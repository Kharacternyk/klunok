#include "extension.h"
#include <assert.h>
#include <string.h>

void test_extension() {
  assert(!strcmp(get_file_extension("/abc/def/archive.tar.gz"), ".tar.gz"));
  assert(!strcmp(get_file_extension("/abc/def/archive"), ""));
  assert(!strcmp(get_file_extension("/abc/def/.archive.tar.gz"), ".tar.gz"));
  assert(!strcmp(get_file_extension("/abc/def/.archive"), ""));
  assert(!strcmp(get_file_extension("archive.tar.gz"), ".tar.gz"));
  assert(!strcmp(get_file_extension("archive"), ""));
  assert(!strcmp(get_file_extension(".archive.tar.gz"), ".tar.gz"));
  assert(!strcmp(get_file_extension(".archive"), ""));
  assert(!strcmp(get_file_extension("mp2.mp3"), ".mp3"));
}
