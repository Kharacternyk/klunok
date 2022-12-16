#include "../src/config.h"
#include "assert.h"

int main() { assert(is_in_set("vim", get_editors())); }
