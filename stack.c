#include <stdlib.h>
#include <stdio.h>
#include "stack.h"

/*
    Pushes item i onto stack s and returns the new top of the stack
*/
STACK* stack_push(STACK* s, void* i)
{
    STACK* top = malloc(sizeof(STACK));
    top->node = i;
    top->next = s;
    return top;
}

/*
    Pops an item from stack s and returns the new top of the stack
*/
STACK* stack_pop(STACK* s)
 {
    if(!s) {
        return NULL;
    }

    STACK* next = s->next;
    free(s);

    return next;
}

/*
    Returns the value of the node at the top of the stack
*/
void* stack_peek(STACK* s) {
    return s ? s->node : NULL;
}