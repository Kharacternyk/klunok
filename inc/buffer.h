#include <stddef.h>

struct trace;

struct buffer *create_buffer(struct trace *trace)
    __attribute__((warn_unused_result));
void concat_string(const char *string, struct buffer *buffer,
                   struct trace *trace);
void concat_char(char c, struct buffer *buffer, struct trace *trace);
void concat_size(size_t size, struct buffer *buffer, struct trace *trace);
void set_length(size_t length, struct buffer *buffer) __attribute__((nonnull));

const struct buffer_view *get_view(const struct buffer *buffer)
    __attribute__((pure, returns_nonnull, nonnull, warn_unused_result));
const char *get_string(const struct buffer_view *view)
    __attribute__((pure, returns_nonnull, nonnull, warn_unused_result));
size_t get_length(const struct buffer_view *view)
    __attribute__((pure, nonnull, warn_unused_result));
size_t get_hash(const struct buffer_view *view)
    __attribute__((pure, nonnull, warn_unused_result));

struct buffer_view *create_buffer_view(const char *string, struct trace *trace)
    __attribute__((warn_unused_result));
void free_buffer_view(struct buffer_view *view);

char *free_outer_buffer(struct buffer *buffer)
    __attribute__((warn_unused_result));
void free_buffer(struct buffer *buffer);
