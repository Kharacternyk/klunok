struct trace;

struct mountinfo *load_mountinfo(struct trace *trace);
char *make_mount(const char *path, const struct mountinfo *mountinfo,
                 struct trace *trace);
void free_mountinfo(struct mountinfo *mountinfo);
