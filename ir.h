#ifndef IR_H
#define IR_H

#include "types.h"
#include "node.h"

typedef struct ir {
    int index;
    int op;
    NODE* lhs;
    NODE* rhs;
} IR;

typedef struct ir_table {
    IR** entries;
    int capacity;
    int next_instruction;
} IRTABLE;

IRTABLE* new_ir_table(int);
NODE* add_instruction(IRTABLE*, int, NODE*, NODE*);

#endif