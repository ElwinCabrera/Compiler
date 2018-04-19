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
    SYMTYPE* tx = expression_type(x);
    if(!type_check_unary_expression(op, x)) {
        invalid_unary_expression(op, tx ? tx->name : "NULL");
    }
    SYMTYPE* result_type = lval_type(op, tx, NULL);
    ADDRESS* a = temp_address(result_type);
    TAC* code = new_tac(op, exp_rvalue(x), NULL, a);
    return temp_expression(add_code(code_table, code));
}

EXPRESSION* binary_expression(TAC_OP op, EXPRESSION* x, EXPRESSION* y) {
    INTERMEDIATE_CODE* code_table = get_intermediate_code();
    SYMTYPE* tx = expression_type(x);
    SYMTYPE* ty = expression_type(y);
    TC_RESULT r = type_check_binary_expression(op, x, y);

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


TC_RESULT type_check_unary_expression(int op, EXPRESSION* x) {
    SYMTYPE* lhs = expression_type(x);
    switch(op) {
        case I_NOT:
            return check_typename(lhs, "Boolean") ? PASS : FAIL;
        case I_REAL2INT: 
            return check_typename(lhs, "real") ? PASS : FAIL;
        case I_INT2REAL:
            return check_typename(lhs, "integer") ? PASS : FAIL;
        case I_SUB:
            return (check_typename(lhs, "integer") | check_typename(lhs, "real")) ? PASS : FAIL;
        case I_IS_NULL:
            return (x->meta == E_ASSIGNABLE && x->value.assignable->meta != A_VARIABLE) ? PASS : FAIL;
        case I_RESERVE:
        case I_RELEASE:
        {
            return (check_metatype(lhs, MT_ARRAY) || check_metatype(lhs, MT_RECORD)) ? PASS : FAIL;
        }
        default:
            return FAIL;
    }
}

TC_RESULT type_check_binary_expression(int op, EXPRESSION* x, EXPRESSION* y) {
    SYMTYPE* lhs = expression_type(x);
    SYMTYPE* rhs = expression_type(y);

    switch(op) {
        case I_AND:
        case I_OR:
            if(check_typename(lhs, "Boolean") && check_typename(rhs, "Boolean")) {
                return PASS;
            }
            return FAIL;
        case I_MODULUS:
            if(check_typename(lhs, "integer") && check_typename(rhs, "integer")) {
                return PASS;
            }
            return FAIL;
        case I_SUB:
        case I_ADD:
        case I_MULTIPLY:
        case I_DIVIDE:
        {
            int left = check_typename(lhs, "integer") ? 1 : check_typename(lhs, "real") ? 2 : 0;
            int right = check_typename(rhs, "integer") ? 1 : check_typename(rhs, "real") ? 2 : 0;
            if(!left || !right) {
                return FAIL;
            }

            if(left > right) {
                return COERCE_RHS;
            } else if(right > left) {
                return COERCE_LHS;
            }

            return PASS;
        }
        case I_LESS_THAN:
        {
            int left = check_typename(lhs, "integer") ? 1 : check_typename(lhs, "real") ? 2 : 
                check_typename(lhs, "Boolean") ? -1 : check_typename(lhs, "character") ? -2 : 0;
            int right = check_typename(rhs, "integer") ? 1 : check_typename(rhs, "real") ? 2 :
                check_typename(lhs, "Boolean") ? -1 : check_typename(lhs, "character") ? -2 : 0;
            
            if(!left || !right) {
                return FAIL;
            }

            if(left < 0 || right < 0) {
                if(left == right) {
                    return PASS;
                }
                return FAIL;
            }

            if(left > right) {
                return COERCE_RHS;
            } else if(right > left) {
                return COERCE_LHS;
            }
        }
        case I_EQUAL: {
            if(lhs && lhs == rhs) {
                return PASS;
            } else {
                int left = check_typename(lhs, "integer") ? 1 : check_typename(lhs, "real") ? 2 : 0;
                int right = check_typename(rhs, "integer") ? 1 : check_typename(rhs, "real") ? 2 : 0;
                if(!left || !right) {
                    if(check_typename(lhs, "nullconst")) {
                        if(check_metatype(rhs, MT_PRIMITIVE)) {
                            return PASS;
                        }
                    } else if(check_typename(rhs, "nullconst")) {
                        if(check_metatype(lhs, MT_PRIMITIVE)) {
                            return PASS;
                        }
                    }

                    return FAIL;
                }                
            }
        }
        default: {
            return FAIL;
        }
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