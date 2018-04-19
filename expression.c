#include <stdlib.h>
#include "intermediate_code.h"
#include "expression.h"
#include "address.h"
#include "assignable.h"
#include "types.h"

extern void invalid_unary_expression(int, char*);
extern void invalid_binary_expression(int, char*,char*);
extern void yyerror(char*);

EXPRESSION* temp_expression(ADDRESS* a) {
    if(!a) {
        return NULL;
    }

    EXPRESSION* e = malloc(sizeof(EXPRESSION));
    e->meta = E_TEMPORARY;
    e->value.temporary = a;
    return e;
}
EXPRESSION* const_expression(ADDRESS* a) {
    if(!a) {
        return NULL;
    }
    EXPRESSION* e = malloc(sizeof(EXPRESSION));
    e->meta = E_CONSTANT;
    e->value.constant = a;
    return e;
}
EXPRESSION* assignable_expression(ASSIGNABLE* a) {
    if(!a) {
        return NULL;
    }
    EXPRESSION* e = malloc(sizeof(EXPRESSION));
    e->meta = E_ASSIGNABLE;
    e->value.assignable = a;
    return e;
}

SYMTYPE* expression_type(EXPRESSION* e) {
    if(!e) {
        return NULL;
    }

    switch(e->meta) {
        case E_TEMPORARY:
            return e->value.temporary->type;
        case E_CONSTANT:
            return e->value.constant->type;
        case E_ASSIGNABLE:
            switch(e->value.assignable->meta) {
                case A_VARIABLE:
                    return e->value.assignable->variable->type;
                case A_RECORD:
                    return e->value.assignable->variable->type;
                case A_ARRAY:
                    return e->value.assignable->array->type;
            }
        default:
            return NULL;
    }
}


EXPRESSION* unary_expression(TAC_OP op, EXPRESSION* x) {
    INTERMEDIATE_CODE* code_table = get_intermediate_code();
    SYMTYPE* t = expression_type(x);
    if(!type_check_unary_expression(op, t)) {
        invalid_unary_expression(op, t ? t->name : "NULL");
    }
    SYMTYPE* result_type = lval_type(op, t, NULL);
    ADDRESS* a = temp_address(result_type);
    TAC* code = new_tac(op, exp_rvalue(x), NULL, a);
    return temp_expression(add_code(code_table, code));
}

EXPRESSION* binary_expression(TAC_OP op, EXPRESSION* x, EXPRESSION* y) {
    INTERMEDIATE_CODE* code_table = get_intermediate_code();
    SYMTYPE* tx = expression_type(x);
    SYMTYPE* ty = expression_type(y);
    TC_RESULT r = type_check_binary_expression(op, tx, ty);

    switch(r) {
        case FAIL:
            invalid_binary_expression(op, tx ? tx->name : "NULL",
                ty ? ty->name : "NULL");
            return temp_expression(NULL);
        case PASS: {
            SYMTYPE* result_type = lval_type(op, tx, ty);
            TAC* code = new_tac(op, exp_rvalue(x), exp_rvalue(y), temp_address(result_type));
            return temp_expression(add_code(code_table, code));
        }
        case COERCE_RHS: {
            TAC* conversion = new_tac(I_INT2REAL, exp_rvalue(y), NULL, temp_address(tx));
            ADDRESS* a = add_code(code_table, conversion);
            TAC* code = new_tac(op, exp_rvalue(x), a, temp_address(tx));
            return temp_expression(add_code(code_table, code));
        }
        case COERCE_LHS: {
            TAC* conversion = new_tac(I_INT2REAL, exp_rvalue(x), NULL, temp_address(ty));
            ADDRESS* a = add_code(code_table, conversion);
            TAC* code = new_tac(op, a, exp_rvalue(y), temp_address(ty));
            return temp_expression(add_code(code_table, code));
        }
        default:
            yyerror("Fatal error parsing a binary expression\n.");
            return temp_expression(NULL);
    }
}

ADDRESS* exp_lvalue(EXPRESSION* e) {
    return NULL;
}

ADDRESS* exp_rvalue(EXPRESSION* e) {
    if(!e) {
        return NULL;
    }

    switch(e->meta) {
        case E_TEMPORARY:
            return e->value.temporary;
        case E_CONSTANT:
            return e->value.constant;
        case E_ASSIGNABLE:
            return assignable_rvalue(e->value.assignable);
        default:
            return NULL;
    }

    return NULL;
}