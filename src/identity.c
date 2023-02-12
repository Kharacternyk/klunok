#include "identity.h"
#include <pwd.h>

struct identity *create_identity(const char *user_name, struct trace *trace) {
  /*FIXME: MT-Unsafe*/
  struct passwd *passwd = getpwnam(user_name);
}
