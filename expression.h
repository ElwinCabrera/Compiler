#ifndef EXPRESSION_H
#define EXPRESSION_H

enum TAC_OP;

typedef enum etype {
    E_TEMPORARY,
    E_CONSTANT,
    E_ASSIGNABLE,
} ETYPE;


typedef enum tc_result {
    FAIL,
    PASS,
    COERCE_LHS,
    COERCE_RHS,
} TC_RESULT;


typedef struct expression {
    ETYPE meta;
    union {
        struct address* temporary;
        struct address* constant;
        struct assignable* assignable;
    } value;
} EXPRESSION;

EXPRESSION* temp_expression(struct address*);
EXPRESSION* const_expression(struct address*);
EXPRESSION* assignable_expression(struct assignable*);
EXPRESSION* binary_expression(TAC_OP, EXPRESSION*, EXPRESSION*);
EXPRESSION* unary_expression(TAC_OP, EXPRESSION*);
struct symtype* expression_type(EXPRESSION*);
TC_RESULT type_check_binary_expression(int, EXPRESSION*, EXPRESSION*);
TC_RESULT type_check_unary_expression(int, EXPRESSION*);

struct address* exp_rvalue(EXPRESSION*);
struct address* exp_lvalue(EXPRESSION*);

#endif