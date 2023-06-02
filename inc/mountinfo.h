struct trace;

struct mountinfo *load_mountinfo(struct trace *trace)
    __attribute__((warn_unused_result));
char *make_mount(const char *path, const struct mountinfo *mountinfo,
                 struct trace *trace) __attribute__((warn_unused_result));
void free_mountinfo(struct mountinfo *mountinfo);
