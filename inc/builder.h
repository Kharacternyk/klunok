#include "trace.h"

struct builder;

struct builder *create_builder(struct trace *trace);
void append_to_builder(const char *string, struct builder *builder,
                       struct trace *trace);
const char *build_string(const struct builder *builder);
void free_builder(struct builder *builder);
