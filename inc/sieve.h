#include <stddef.h>

struct set;
struct trace;

struct sieved_path *sieve(const char *path, size_t relative_path_offset,
                          const struct set **sets, size_t set_count,
                          struct trace *trace);

const char *get_hiding_dot(const struct sieved_path *sieved_path);
const char *const *get_sieved_ends(const struct sieved_path *sieved_path);

void free_sieved_path(struct sieved_path *sieved_path);
