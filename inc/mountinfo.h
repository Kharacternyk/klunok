struct trace;

struct mountinfo *create_mountinfo(struct trace *trace);
const char *get_next_block_mount(struct mountinfo *mountinfo);
void free_mountinfo(struct mountinfo *mountinfo);
