#include <stdio.h>
#include "ir.h"

IRTABLE* new_ir_table(int capacity) {
    IRTABLE* irtab = malloc(sizeof(IRTABLE));
    irtab->entries = malloc(sizeof(IR) * capacity);
    irtab->capacity = capacity;
    return irtab;
}

char* lval_type(INSTRUCTION op, NODE* lhs, NODE* rhs) {
    switch(op) {
        case I_LOOKUP:
        case I_ASSIGN:
            return lhs->type_name;
        case I_IS_NULL:
        case I_NOT:
        case I_AND:
        case I_OR:
        case I_LESS_THAN:
        case I_EQUAL:
            return "Boolean";
        case I_REAL2INT:
        case I_MODULUS:
            return "integer";
        case I_INT2REAL:
            return "real";
        case I_MULTIPLY:
        case I_DIVIDE:
        case I_ADD:
        case I_SUB:
            return lhs->type_name;
        default:
            return NULL;
    }
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
    
    return ir_node(i->index, lval_type(op, lhs, rhs));
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

void parse_ir(IR_PRINT* istr, IR* i) {

}