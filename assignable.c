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
extern void invalid_unary_expression(int, char*);
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
    
    if(!array) {
        return NULL;
    }
    
    int actual = 0;
    int expected = array->type->dimensions;
    int cur = array->type->dimensions;

    int result_width = 0;

    while(indices) {
        EXPRESSION* index = stack_peek(indices);
        SYMTYPE* t = expression_type(index);
        if(check_typename(t, "integer")) {
            ADDRESS* a = exp_rvalue(index);
            result_width += a->value.integer * cur;
        } else {
            type_mismatch_error("integer", t ? t->name : NULL);
        }
        cur--;
        actual++;
        indices = stack_pop(indices);
    }

    if(actual != expected) {
        argument_count_mismatch(expected, actual);
        return NULL;
    }

    ASSIGNABLE* assign = new_assignable();
    assign->meta = A_ARRAY;
    assign->array = array;
    assign->array_offset = result_width;

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
        SYMBOL* param = ll_value(params);
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

    switch(a->meta) {
        case A_VARIABLE: {
            if(a->variable->type && a->variable->type->meta == MT_FUNCTION) {
                return symbol_address(a->variable->type->ret);
            }
            return a->variable;
        }
        case A_ARRAY: {
             return a->array;
        }
        case A_RECORD: {
            return a->record;
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
        case A_RECORD: {
            INTERMEDIATE_CODE* code_table = get_intermediate_code();
            ADDRESS* result = temp_address(a->variable->type);
            TAC* access = new_tac(I_RECORD_ACCESS, a->record, a->variable, result);
            return add_code(code_table, access);
        }
        case A_ARRAY: {
            INTERMEDIATE_CODE* code_table = get_intermediate_code();
            ADDRESS* result = temp_address(a->array->type->element_type);
            TAC* access = new_tac(I_ARRAY_ACCESS, a->array, int_address(a->array_offset), result);
            return add_code(code_table, access);
        }
        default:
            return NULL;
    }
}

ADDRESS* handle_assignment(ASSIGNABLE* a, EXPRESSION* e) {
    
    if(!a) {
        return NULL;
    }

    
    INTERMEDIATE_CODE* code_table = get_intermediate_code();
    ADDRESS* adr = assignable_lvalue(a);

    SYMTYPE* a_type = adr ? adr->type : NULL;
    SYMTYPE* e_type = expression_type(e);
    ADDRESS* result = NULL;
    bool type_error = false;
    if(a->meta == A_ARRAY) {
        if(a_type->element_type != e_type && !check_typename(e_type, "nullconst")) {
           type_error = true;
        }

        TAC* array = new_tac(I_ARRAY_ASSIGN, int_address(a->array_offset), exp_rvalue(e), a->array);
        result = add_code(code_table, array);
    } else if(a->meta == A_RECORD) {
        if(a->variable->type != e_type && !check_typename(e_type, "nullconst")) {
           type_error = true;
        }
        TAC* rec = new_tac(I_RECORD_ASSIGN, a->variable, exp_rvalue(e), a->record);
        result = add_code(code_table, rec);
    } else {
        if(a_type != e_type) {
            type_error = true;
        }
        TAC* var = new_tac(I_ASSIGN, exp_rvalue(e), NULL, adr);
        result = add_code(code_table, var);
        
    }

    if(type_error) {
        type_mismatch_error(a_type ? a_type->name : "NULL", e_type ? e_type->name : "NULL");
    }

    return result;
}

void handle_memop(TAC_OP op, ASSIGNABLE* a) {
    
    INTERMEDIATE_CODE* code_table = get_intermediate_code();
    ADDRESS* adr = assignable_lvalue(a);
    ADDRESS* width = NULL;

    if(!adr) {
        invalid_unary_expression(op, "NULL");
    } else if(check_metatype(adr->type, MT_RECORD)) {
        int calc_width = 0;
        LINKED_LIST* symbols = adr->type->members->symbols;
        while(symbols) {
            SYMBOL* s = ll_value(symbols);
            calc_width += get_type_width(s->type);
            symbols = ll_next(symbols);
        }
        adr->value.symbol->width = calc_width;
        width = int_address(calc_width);
        add_code(code_table, new_tac(op, width, NULL, adr));

    } else {
        if(op == I_RESERVE) {
            int element_width = get_type_width(adr->type->element_type);
            int base_width = 4 + 4 * adr->type->dimensions;
            width = int_address(element_width * a->array_offset + base_width);
        } else {
            // TODO
            width = int_address(0);
        }

        add_code(code_table, new_tac(op, width, NULL, adr));
    }
}