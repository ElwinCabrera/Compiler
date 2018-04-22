#include <stdlib.h>
#include "types.h"
#include "assignable.h"
#include "address.h"
#include "stack.h"
#include "symbol_table.h"
#include "intermediate_code.h"
#include "expression.h"
#include "linked_list.h"

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
    LINKED_LIST* params = fn->type->parameters->symbols;
    int expected = 0;
    int actual = 0;
    while(params) {
        SYMTAB* param = ll_value(params);
        if(!args) {
            while(params) { params = ll_next(params); expected++; }
            argument_count_mismatch(expected, actual);
            break;
        }
        actual++;
        EXPRESSION* e = stack_peek(args);
        SYMTYPE* actual = expression_type(e);
        
        if(param->type && param->type == actual) {
            add_code(code_table, new_tac(I_PARAM, exp_rvalue(e), NULL, NULL));
        } else {
            type_mismatch_error(param->type ? param->type->name : "NULL", 
                actual ? actual->name : "NULL");
        }

        args = stack_pop(args);
        params = ll_next(params);
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
            INTERMEDIATE_CODE* code_table = get_intermediate_code();
            STACK* i = a->indices;
            ADDRESS* array = assignable_rvalue(a);
            while(i) {
                EXPRESSION* e = stack_peek(i);
                ADDRESS* result = temp_address(a->array->type);
                TAC* code = new_tac(I_ARRAY, array, exp_rvalue(e), result);
                array = add_code(code_table, code);
                i = i->next;
            }
            return array;
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
            return assignable_lvalue(a);
        case A_ARRAY: {
            return symbol_address(a->array->value.symbol);
        }
        default:
            return NULL;
    }
}

void handle_assignment(ASSIGNABLE* a, EXPRESSION* e) {
    INTERMEDIATE_CODE* code_table = get_intermediate_code();
    ADDRESS* adr = assignable_lvalue(a);

    if(adr && adr->type && adr->type->meta == MT_FUNCTION) {
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



int get_width(ADDRESS *adr) {
    int width = 0;
    int dimension = adr->type->dimension;
    if (adr->type->meta == MT_ARRAY){
    
// Probably not the correct calculation
        width = adr->type->element_type->width;
        width = width * dimension;
        return width;
    }
    width = adr->type->width;
    return width;
    
    
}

void handle_memop(TAC_OP op, ASSIGNABLE* a) {
    
    INTERMEDIATE_CODE* code_table = get_intermediate_code();
    ADDRESS* adr = assignable_rvalue(a);
    //int width = get_type_width(adr->type);
    int width = get_width(adr);
    if(op == I_RESERVE) {
        add_code(code_table, new_tac(op, int_address(width), NULL, adr));
    } else {
        add_code(code_table, new_tac(op, adr, int_address(width), NULL));
    }
}