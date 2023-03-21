#include <stddef.h>

struct buffer;
struct trace;

struct buffer *create_buffer(struct trace *trace);
void concat_string(const char *string, struct buffer *buffer,
                   struct trace *trace);
void concat_char(char c, struct buffer *buffer, struct trace *trace);
void concat_size(size_t size, struct buffer *buffer, struct trace *trace);
const char *get_string(const struct buffer *buffer);
size_t get_buffer_length(const struct buffer *buffer);
void truncate_buffer(size_t length, struct buffer *buffer);
char *free_outer_buffer(struct buffer *buffer);
void free_buffer(struct buffer *buffer);
