#include <stdlib.h>
#include "types.h"
#include "assignable.h"
#include "address.h"
#include "stack.h"
#include "symbol_table.h"
#include "intermediate_code.h"
#include "expression.h"

extern void argument_count_mismatch(int, int);
extern void type_mismatch_error(char*, char*);

ASSIGNABLE* new_assignable() {
    return malloc(sizeof(ASSIGNABLE));
}

ASSIGNABLE* assignable_variable(ADDRESS* a) {
    ASSIGNABLE* assign = new_assignable();
    assign->meta = A_VARIABLE;
    assign->variable = a;
    return assign;
}

ASSIGNABLE* assignable_record(ADDRESS* record, ADDRESS* member) {
    ASSIGNABLE* assign = new_assignable();
    assign->meta = A_RECORD;
    assign->variable = member;
    assign->record = record;
    return assign;
}

ASSIGNABLE* assignable_array(ADDRESS* array, STACK* indices) {
    int actual = 0;
    int expected = array->type->dimensions;
    STACK* check_indices = indices;

    while(check_indices) {
        actual++;
        EXPRESSION* index = stack_peek(check_indices);
        SYMTYPE* actual = expression_type(index);
        if(!check_typename(actual, "integer")) {
            type_mismatch_error("integer", actual ? actual->name : NULL);
        }
        check_indices = check_indices->next;
    }

    if(actual != expected) {
        argument_count_mismatch(expected, actual);
        return NULL;
    }

    ASSIGNABLE* assign = new_assignable();
    assign->meta = A_ARRAY;
    assign->array = array;
    assign->indices = indices;

    return assign;
}

ASSIGNABLE* assignable_function(ADDRESS* fn, STACK* args) {
    
    if(!fn) {
        return NULL;
    }

    INTERMEDIATE_CODE* code_table = get_intermediate_code();
    SYMTAB* params = fn->type->parameters->symbols;
    int expected = 0;
    int actual = 0;
    while(params) {
        if(!args) {
            while(params) { params = params->next; expected++; }
            argument_count_mismatch(expected, actual);
            break;
        }
        actual++;
        EXPRESSION* e = stack_peek(args);
        SYMTYPE* actual = expression_type(e);
        
        if(params->type && params->type == actual) {
            add_code(code_table, new_tac(I_PARAM, exp_rvalue(e), NULL, NULL));
        } else {
            type_mismatch_error(params->type ? params->type->name : "NULL", 
                actual ? actual->name : "NULL");
        }

        args = stack_pop(args);
        params = params->next;
        expected++;
    }

    if(args) {
        while(args) { args = stack_pop(args); actual++; }
        argument_count_mismatch(expected, actual);
    }

    ADDRESS* arg_count = int_address(expected);
    ADDRESS* ret_temp = temp_address(fn->type->ret->type);
    TAC* code = new_tac(I_CALL, fn, arg_count, ret_temp);
    return assignable_variable(add_code(code_table, code));
}

/*
    Assignable on the left side of an assignment
    Need to write to its memory address

*/
ADDRESS* assignable_lvalue(ASSIGNABLE* a) {
    if(!a) {
        return NULL;
    }

    INTERMEDIATE_CODE* code_table = get_intermediate_code();

    switch(a->meta) {
        case A_VARIABLE:
            return a->variable;
        case A_ARRAY: {
            /* 
                This is an array access, we have the indicies in a stack:
                a->indicies
            */
            ADDRESS* result = temp_address(a->array->type->element_type);
            TAC* code = new_tac(I_ARRAY, a->array, int_address(0), result);
            return add_code(code_table, code);
        }
        case A_RECORD: {
            ADDRESS* result = temp_address(a->variable->type);
            TAC* code = new_tac(I_LOOKUP, a->record, a->variable, result);
            return add_code(code_table, code);
        default:
            return NULL;
        }

    }
}

/*
    Assignable on the right side of an assignment
    This is a lookup/access
*/
ADDRESS* assignable_rvalue(ASSIGNABLE* a) {
    if(!a) {
        return NULL;
    }

    switch(a->meta) {
        case A_VARIABLE:
            return a->variable;
        case A_RECORD:
        case A_ARRAY: {
            return assignable_lvalue(a);
        default:
            return NULL;
        }
    }
}

void handle_assignment(ASSIGNABLE* a, EXPRESSION* e) {
    INTERMEDIATE_CODE* code_table = get_intermediate_code();
    ADDRESS* adr = assignable_lvalue(a);

    if(adr && adr->type->meta == MT_FUNCTION) {
        adr = symbol_address(adr->type->ret);
    }

    SYMTYPE* a_type = adr ? adr->type : NULL;
    SYMTYPE* e_type = expression_type(e);
    
    if(!(a_type && (a_type == e_type)) && 
        !((check_typename(e_type, "nullconst") &&
        (a->meta == A_ARRAY || a->meta == A_RECORD)))
    ) {
        type_mismatch_error(a_type ? a_type->name : "NULL", e_type ? e_type->name : "NULL");
    }

    add_code(code_table, new_tac(I_ASSIGN, adr, exp_rvalue(e), NULL));
}