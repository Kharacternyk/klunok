#ifndef CALLBACK_H_INCLUDED
#define CALLBACK_H_INCLUDED

struct callback;

struct callback *create_callback(void (*function)(void *parameter),
                                 void *parameter);

void invoke_callback(struct callback *callback);

#endif
