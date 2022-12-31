#ifndef CALLBACK_H_INCLUDED
#define CALLBACK_H_INCLUDED
#include <stdbool.h>

struct callback;

struct callback *create_callback(void (*function)(void *parameter),
                                 void *parameter, struct callback *next);
struct callback *create_simple_callback(bool *is_error, struct callback *next);

void invoke_callback(struct callback *callback);

#endif
