#ifndef IR_H
#define IR_H

#include "types.h"
#include "node.h"

typedef enum instruction {
    I_ASSIGN,
    I_LOOKUP,
    I_ADD,
    I_SUB,
    I_MULTIPLY,
    I_DIVIDE,
    I_MODULUS,
    I_LESS_THAN,
    I_EQUAL,
    I_REAL2INT,
    I_INT2REAL,
    I_IS_NULL,
    I_NOT,
    I_AND,
    I_OR,
    I_PARAM,
    I_CALL,
    I_RETURN,
    I_TEST,
    I_TEST_FALSE,
    I_GOTO,
} INSTRUCTION;

typedef struct ir {
    int index;
    INSTRUCTION op;
    NODE* lhs;
    NODE* rhs;
} IR;

typedef struct ir_table {
    IR** entries;
    int capacity;
    int next_instruction;
} IRTABLE;

IRTABLE* new_ir_table(int);
NODE* add_instruction(IRTABLE*, INSTRUCTION, NODE*, NODE*);

#endif