#include "config.h"

struct set *get_editors() {
  struct set *editors = create_set();

  add_to_set("nvim", editors);
  add_to_set("vim", editors);
  add_to_set("vi", editors);

  return editors;
}

const char *get_store_path() { return "/klunok"; }
