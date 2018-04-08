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
    
    if(!irtab) {
        return NULL;
    }

    IR* i = new_instruction(irtab->next_instruction, op, lhs, rhs);
    irtab->entries[irtab->next_instruction++] = i;
    
    if(lhs) {
        return ir_node(i->index, lhs->type_name);
    }

    return ir_node(i->index, NULL);
}

void print_ir_table(IRTABLE* irtab, FILE* f) {
    
    if(!irtab) {
        return;
    }

    for(int i = 0; i < irtab->next_instruction; i++) {
        print_ir(irtab->entries[i], f);
    }
}

void print_ir(IR* i, FILE* f) {
    
    if(!i) {
        return;
    }

    if(f) {
        fprintf(f, "[%d] OP: %d, LHS: %p, RHS: %p\n", i->index, i->op, i->lhs, i->rhs);
    } else {
        printf("[%d] OP: %d, LHS: %p, RHS: %p\n", i->index, i->op, i->lhs, i->rhs);
    }
}