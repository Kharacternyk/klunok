struct trace;

struct store_path *create_store_path(const char *root,
                                     const char *relative_path,
                                     const char *version, struct trace *trace);
const char *get_current_path(const struct store_path *store_path)
    __attribute__((pure, returns_nonnull, nonnull, warn_unused_result));
void increment(struct store_path *store_path, struct trace *trace);
void free_store_path(struct store_path *store_path);
