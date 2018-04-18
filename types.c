#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "intermediate_code.h"
#include "symbol_table.h"
#include "types.h"

static TYPE_CONTAINER* known_types;

TYPE_CONTAINER* get_type_container() {
    if(!known_types) {
        known_types = malloc(sizeof(TYPE_CONTAINER));
    } 

    return known_types;
}

SYMTYPE* add_type(TYPE_CONTAINER* container, TTYPE meta, char* name) {

    if(!name || find_type(container, name)) {
        return NULL;
    }

    SYMTYPE* new = malloc(sizeof(SYMTYPE));

    new->meta = meta;
    new->name = name;
    new->next = container->head;

    container->head = new;

    return new;
}

SYMTYPE* find_type(TYPE_CONTAINER* container, char* name) {

    if(!name || !container) {
        return NULL;
    }

    SYMTYPE* s = container->head;

    while(s) {
        if(strcmp(name, s->name) == 0) {
            return s;
        }
        s = s->next;
    }

    return NULL;
}

bool check_metatype(SYMTYPE* t, TTYPE meta) {

    if (!t) {
        return false;
    }

    return t->meta == meta;
}

bool check_typename(SYMTYPE* t, char* name) {

    if (!t || !name) {
        return false;
    }

    return strcmp(name, t->name) == 0;
}

bool compare_typenames(char* t1, char* t2) {
    if(!t1 || !t2) {
        return false;
    }

    return strcmp(t1, t2) == 0;
}

SYMTYPE* lval_type(TAC_OP op, SYMTYPE* lhs, SYMTYPE* rhs) {
    switch(op) {
        case I_LOOKUP:
        case I_ASSIGN:
            return lhs;
        case I_IS_NULL:
        case I_NOT:
        case I_AND:
        case I_OR:
        case I_LESS_THAN:
        case I_EQUAL:
            return find_type(known_types, "Boolean");
        case I_REAL2INT:
        case I_MODULUS:
            return find_type(known_types, "integer");
        case I_INT2REAL:
            return find_type(known_types, "real");
        case I_MULTIPLY:
        case I_DIVIDE:
        case I_ADD:
        case I_SUB:
            return lhs;
        case I_ARRAY:
            return lhs->element_type;
        case I_CALL:
            return lhs->ret->type;
        default:
            return NULL;
    }
}

TC_RESULT type_check_unary_expression(int op, SYMTYPE* lhs) {
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
        case I_RESERVE:
        case I_RELEASE:
        {
            return (check_metatype(lhs, MT_ARRAY) || check_metatype(lhs, MT_RECORD)) ? PASS : FAIL;
        }
        default:
            return FAIL;
    }
}

TC_RESULT type_check_binary_expression(int op, SYMTYPE* lhs, SYMTYPE* rhs) {
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