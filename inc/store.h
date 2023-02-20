struct trace;

void copy_to_store(const char *path, const char *version,
                   const char *store_root, struct trace *trace);

void copy_delta_to_store(const char *path, const char *version,
                         const char *cursor_name, const char *store_root,
                         struct trace *trace);
