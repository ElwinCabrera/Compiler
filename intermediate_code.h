#ifndef INTERMEDIATE_CODE_H
#define INTERMEDIATE_CODE_H

#include <stdio.h>
#include <stdbool.h>

typedef enum tac_op {
    I_ASSIGN,
    I_RECORD_ASSIGN,
    I_RECORD_ACCESS,
    I_ARRAY_ASSIGN,
    I_ARRAY_ACCESS,
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
    I_TEST_NOTEQUAL,
    I_GOTO,
    I_RESERVE,
    I_RELEASE,
    I_NOP,
    I_FN_START,
    I_STACK_VARS,
    I_VALNUM_NODE,
    I_VALNUM_LINKED,
	//ADDED a new TAC OP in order to work with VALNUM
} TAC_OP;

typedef struct tac {
    int label;
    bool leader;
    TAC_OP op;
    int result_next;
    int x_next;
    int y_next;
    bool result_live;
    bool x_live;
    bool y_live;
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
bool code_is_jump(TAC*);

#endif