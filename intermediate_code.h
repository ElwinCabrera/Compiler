#ifndef INTERMEDIATE_CODE_H
#define INTERMEDIATE_CODE_H

#include <stdio.h>

typedef enum tac_op {
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
    I_ARRAY,
    I_RETURN,
    I_TEST,
    I_TEST_FALSE,
    I_TEST_NOTEQUAL,
    I_GOTO,
    I_RESERVE,
    I_RELEASE,
    I_NOP,
} TAC_OP;

typedef struct tac {
    int label;
    TAC_OP op;
    struct address* result;
    struct address* x;
    struct address* y;
    char* string;
} TAC;

typedef struct intermediate_code {
    struct tac** entries;
    int next_instruction;
    int capacity;
} INTERMEDIATE_CODE;

INTERMEDIATE_CODE* get_intermediate_code();
struct address* add_code(INTERMEDIATE_CODE*, TAC*);
TAC* new_tac(TAC_OP, struct address*, struct address*, struct address*);
void backpatch(INTERMEDIATE_CODE*, int, int);
void print_intermediate_code(INTERMEDIATE_CODE*, FILE*);
void print_tac(TAC*, FILE*);
const char* get_op_string(TAC_OP);

#endif