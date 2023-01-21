#include "extension.h"
#include <assert.h>
#include <string.h>

int main() {
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
