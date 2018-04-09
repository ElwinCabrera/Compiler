#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "types.h"
#include "ir.h"

extern SYMTYPE** get_types();

SYMTYPE * add_type(SYMTYPE* start, TYPEMETA meta, char* name) {

    if(!name || find_type(start, name)) {
        return NULL;
    }

    SYMTYPE * new = malloc(sizeof(SYMTYPE));

    switch(meta) {
        case MT_FUNCTION:
            new->details.function = malloc(sizeof(struct function_details));
            break;
        case MT_RECORD:
            new->details.record = malloc(sizeof(struct record_details));
            break;
        case MT_ARRAY:
            new->details.array = malloc(sizeof(struct array_details));
            break;
        default:
            new->details.primitive = NULL;
            break;
    }

    new->meta = meta;
    new->name = name;
    new->next = start;

    return new;
}

SYMTYPE * find_type(SYMTYPE* start, char* name) {

    if(!name) {
        return NULL;
    }

    SYMTYPE * s = start;

    while(s) {
        if(strcmp(name, s->name) == 0) {
            return s;
        }
        s = s->next;
    }

    return NULL;
}

bool check_metatype(SYMTYPE* t, TYPEMETA meta) {

    if (!t) {
        return false;
    }

    return t->meta == meta;
}

bool check_type(SYMTYPE* t, char* name) {

    if (!t || !name) {
        return false;
    }

    return strcmp(name, t->name) == 0;
}

bool compare_types(char* t1, char* t2) {
    if(!t1 || !t2) {
        return false;
    }

    return strcmp(t1, t2) == 0;
}

TC_RESULT type_check_unary_expression(int op, char* lhs) {
    switch(op) {
        case I_NOT:
            return compare_types(lhs, "Boolean") ? PASS : FAIL;
        case I_REAL2INT:
            return compare_types(lhs, "real") ? PASS : FAIL;
        case I_INT2REAL:
            return compare_types(lhs, "integer") ? PASS : FAIL;
        case I_SUB:
            return (compare_types(lhs, "integer") | compare_types(lhs, "real")) ? PASS : FAIL;
        case I_IS_NULL:
        case I_RESERVE:
        case I_RELEASE:
        {
            SYMTYPE* t = find_type(*get_types(), lhs);
            return (t && (t->meta == MT_ARRAY || t->meta == MT_RECORD)) ? PASS : FAIL;
        }
        default:
            return FAIL;
    }
}

TC_RESULT type_check_binary_expression(int op, char* lhs, char* rhs) {
    switch(op) {
        case I_AND:
        case I_OR:
            if(compare_types(lhs, "Boolean") && compare_types(rhs, "Boolean")) {
                return PASS;
            }
            return FAIL;
        case I_MODULUS:
            if(compare_types(lhs, "integer") && compare_types(rhs, "integer")) {
                return PASS;
            }
            return FAIL;
        case I_SUB:
        case I_ADD:
        case I_MULTIPLY:
        case I_DIVIDE:
        {
            int left = compare_types(lhs, "integer") ? 1 : compare_types(lhs, "real") ? 2 : 0;
            int right = compare_types(rhs, "integer") ? 1 : compare_types(rhs, "real") ? 2 : 0;
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
            int left = compare_types(lhs, "integer") ? 1 : compare_types(lhs, "real") ? 2 : 
                compare_types(lhs, "Boolean") ? -1 : compare_types(lhs, "character") ? -2 : 0;
            int right = compare_types(rhs, "integer") ? 1 : compare_types(rhs, "real") ? 2 :
                compare_types(lhs, "Boolean") ? -1 : compare_types(lhs, "character") ? -2 : 0;
            
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
            if(compare_types(lhs, rhs)) {
                return PASS;
            } else {
                int left = compare_types(lhs, "integer") ? 1 : compare_types(lhs, "real") ? 2 : 0;
                int right = compare_types(rhs, "integer") ? 1 : compare_types(rhs, "real") ? 2 : 0;
                if(!left || !right) {
                    if(compare_types(lhs, "null")) {
                        SYMTYPE* t = find_type(*get_types(), rhs);
                        if(t && t->meta != MT_PRIMITIVE) {
                            return PASS;
                        }
                    } else if(compare_types(rhs, "null")) {
                        SYMTYPE* t = find_type(*get_types(), lhs);
                        if(t && t->meta != MT_PRIMITIVE) {
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