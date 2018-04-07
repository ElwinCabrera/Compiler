#include <stdio.h>
#include "ir.h"

IRTABLE* new_ir_table(int capacity) {
    IRTABLE* irtab = malloc(sizeof(IRTABLE));
    irtab->entries = malloc(sizeof(IR) * capacity);
    irtab->capacity = capacity;
    return irtab;
}

IR* new_instruction(int index, INSTRUCTION op, NODE* lhs, NODE* rhs) {
    IR* i = malloc(sizeof(IR));
    i->index = index;
    i->op = op;
    i->lhs = lhs;
    i->rhs = rhs;
    return i;
}

NODE* add_instruction(IRTABLE* irtab, INSTRUCTION op, NODE* lhs, NODE* rhs) {
    
    if(!irtab || !lhs) {
        return NULL;
    }

    printf("OP: %d, LHS: %p, RHS: %p\n", op, lhs, rhs);
    IR* i = new_instruction(irtab->next_instruction, op, lhs, rhs);
    irtab->entries[irtab->next_instruction++] = i;

    return ir_node(i, lhs->type_name);
}