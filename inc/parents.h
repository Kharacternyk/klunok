struct trace;

void create_parents(const char *path, struct trace *trace);
void remove_empty_parents(const char *path, struct trace *trace);
