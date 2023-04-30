struct trace;

struct list *create_list(struct trace *trace);
void join(const char *head, struct list *tail, struct trace *trace);
const struct list_item *peek(const struct list *list);
const struct list_item *get_next(const struct list_item *item);
const char *get_value(const struct list_item *item);
void free_list(struct list *list);
