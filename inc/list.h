struct trace;

struct list *create_list(struct trace *trace)
    __attribute__((warn_unused_result));
void join(const char *head, struct list *tail, struct trace *trace);
const struct list_item *peek(const struct list *list)
    __attribute__((pure, nonnull, warn_unused_result));
const struct list_item *get_next(const struct list_item *item)
    __attribute__((pure, nonnull, warn_unused_result));
const char *get_value(const struct list_item *item)
    __attribute__((pure, nonnull, warn_unused_result));
void free_list(struct list *list);
