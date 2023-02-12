#include "trace.h"
#include <sys/types.h>

struct identity;

struct identity *create_identity(const char *user_name, struct trace *trace);
uid_t get_identity_uid(const struct identity *identity);
gid_t get_identity_gid(const struct identity *identity);
void free_identity(struct identity *identity);
