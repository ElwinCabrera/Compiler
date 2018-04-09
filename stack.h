#ifndef STACK_H
#define STACK_H

typedef struct stack {
    void* node;
    struct stack* next;
} STACK;

STACK* stack_push(STACK*, void*);
STACK* stack_pop(STACK*);
void* stack_peek();

#endif