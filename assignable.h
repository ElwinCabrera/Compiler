#ifndef ASSIGNABLE_H
#define ASSIGNABLE_H

#include "expression.h"
#include "stack.h"

typedef enum assign_type {
    A_VARIABLE,
    A_RECORD,
    A_ARRAY,
} ASSIGN_TYPE;

typedef struct assignable {
    ASSIGN_TYPE meta;
    struct address* variable;
    struct address* record;
    struct address* array;
    int array_offset;
    
} ASSIGNABLE;

ASSIGNABLE* assignable_variable(struct address*);
ASSIGNABLE* assignable_record(struct address*, struct address*);
ASSIGNABLE* assignable_array(struct address*, struct stack*);
ASSIGNABLE* assignable_function(struct address*, struct stack*);
struct address* assignable_lvalue(ASSIGNABLE*);
struct address* assignable_rvalue(ASSIGNABLE*);
struct address* handle_assignment(ASSIGNABLE*, struct expression*);
void handle_memop(TAC_OP, ASSIGNABLE*);

#endif