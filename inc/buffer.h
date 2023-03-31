#include <stddef.h>

struct trace;

struct buffer *create_buffer(struct trace *trace);
void concat_string(const char *string, struct buffer *buffer,
                   struct trace *trace);
void concat_char(char c, struct buffer *buffer, struct trace *trace);
void concat_size(size_t size, struct buffer *buffer, struct trace *trace);
void set_length(size_t length, struct buffer *buffer);

const struct buffer_view *get_view(const struct buffer *buffer);
const char *get_string(const struct buffer_view *view);
size_t get_length(const struct buffer_view *view);
size_t get_hash(const struct buffer_view *view);

struct buffer_view *create_buffer_view(const char *string, struct trace *trace);
void free_buffer_view(struct buffer_view *view);

char *free_outer_buffer(struct buffer *buffer);
void free_buffer(struct buffer *buffer);
