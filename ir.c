#include <stdio.h>
#include "symbolTable.h"
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
        case I_ARRAY:
            return lhs->value.symbol->type->details.array->element_type->name;
        case I_CALL:
            return lhs->value.symbol->type->details.function->return_type->type->name;
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
        fprintf(f, "[%d] OP: %s, LHS: %s, RHS: %s\n", i->index, get_op_string(i->op), node_to_string(i->lhs), node_to_string(i->rhs));
    } else {
        printf("[%d] OP: %s, LHS: %s, RHS: %s\n", i->index, get_op_string(i->op), node_to_string(i->lhs), node_to_string(i->rhs));
    }
}

const char* get_op_string(INSTRUCTION i) {
    switch(i) {
        case I_ASSIGN:
            return "assign";
        case I_LOOKUP:
            return "lookup";
        case I_ADD:
            return "+";
        case I_SUB:
            return "-";
        case I_MULTIPLY:
            return "*";
        case I_DIVIDE:
            return "/";
        case I_MODULUS:
            return "%";
        case I_LESS_THAN:
            return ">";
        case I_EQUAL:
            return "==";
        case I_REAL2INT:
            return "r2i";
        case I_INT2REAL:
            return "i2r";
        case I_IS_NULL:
            return "isNull";
        case I_NOT:
            return "!";
        case I_AND:
            return "&";
        case I_OR:
            return "|";
        case I_PARAM:
            return "param";
        case I_CALL:
            return "call";
        case I_RETURN:
            return "return";
        case I_TEST:
            return "test";
        case I_TEST_FALSE:
            return "testfalse";
        case I_GOTO:
            return "goto";
        case I_RESERVE:
            return "reserve";
        case I_RELEASE:
            return "release";
        case I_ARRAY:
            return "array";
        default:
            return NULL;
    }
}

void parse_ir(IR_PRINT* istr, IR* i) {

}