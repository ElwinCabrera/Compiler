#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "address.h"
#include "symbol_table.h"
#include "types.h"
#include "assignable.h"
#include "linked_list.h"
#include "data_block.h"


static int temporary_count = 0;

/*
    According to text, an address can be:
        - A symbol (name)
        - A constant
        - A temporary

    This is a generic wrapper for an address creation
*/
ADDRESS* new_address() {
    ADDRESS* a = malloc(sizeof(ADDRESS));
    memset(a, 0, sizeof(ADDRESS));
    return a;
};

/*
    Creates an address for a temporary value.
    Increases the temporary variable counter.
*/
ADDRESS* temp_address(SYMTYPE* t) {
    ADDRESS* a = new_address();
    a->meta = AT_TEMPORARY;
    char name[12];
    sprintf(name, "t%d", temporary_count++);
    a->value.symbol = new_symbol(t, name, ST_TEMPORARY, "generated");
    SYMBOL_TABLE* st = get_symbol_table();
    add_symbols_to_scope(st->current_scope, ll_new(a->value.symbol));
    a->type = t;
    return a;
}

/*
    Creates an address from a symbol table entry
*/
ADDRESS* symbol_address(SYMBOL* s) {
    ADDRESS* a = new_address();
    a->meta = AT_SYMBOL;
    a->value.symbol = s;
    a->type = s ? s->type : NULL;
    return a;
}

/*
    Creates an address for a label
    This is used for branching
*/
ADDRESS* label_address(int n) {
    ADDRESS* a = new_address();
    a->meta = AT_LABEL;
    a->value.label = n;
    a->type = NULL;
    return a;
}

/*
    integer constant
*/
ADDRESS* int_address(int n) {
    ADDRESS* a = new_address();
    a->meta = AT_INT;
    a->value.integer = n;
    a->type = find_type(get_type_container(), "integer");
    return a;
}

/*
    boolean constant
*/
ADDRESS* boolean_address(int b) {
    ADDRESS* a = new_address();
    a->meta = AT_BOOLEAN;
    a->value.boolean = b;
    a->type = find_type(get_type_container(), "Boolean");
    return a;
}

/*
    real constant
*/
ADDRESS* real_address(double d) {
    ADDRESS* a = new_address();
    a->meta = AT_REAL;
    a->value.real = d;
    a->type = find_type(get_type_container(), "real");
    a->block = store_in_data_block(a);
    return a;
};

/*
    char constant
*/
ADDRESS* char_address(char c) {
    ADDRESS* a = new_address();
    a->meta = AT_CHAR;
    a->value.character = c;
    a->type = find_type(get_type_container(), "character");
    return a;
};

/*
    string constant
*/
ADDRESS* string_address(char* s) {
    ADDRESS* a = new_address();
    a->meta = AT_STRING;
    a->value.string = s;
    a->type = find_type(get_type_container(), "string");
    a->block = store_in_data_block(a);
    return a;
};

/*
    null constant
*/
ADDRESS* null_address() {
    ADDRESS* a = new_address();
    a->value.null = 0;
    a->meta = AT_NULL;
    a->type = find_type(get_type_container(), "nullconst");
    return a;
}

ADDRESS* scope_address(SCOPE* s) {
    ADDRESS* a = new_address();
    a->value.scope = s;
    a->meta = AT_SCOPE;
    return a;
}

/*
    Creates a string represenation of an address
    Be sure to free() this
*/
char* create_address_string(ADDRESS* a) {
    if(!a) {
        return NULL;
    }

    switch(a->meta) {
        case AT_STRING:
            return strdup(a->value.string);
        case AT_CHAR: {
            char* tmp = malloc(2 * sizeof(char));
            sprintf(tmp, "'%c'", a->value.character);
            return tmp;
        }
        case AT_INT: {
            char* tmp = malloc(20 * sizeof(char));
            sprintf(tmp, "%d", a->value.integer);
            return tmp;
        }
        case AT_REAL: {
            char* tmp = malloc(20 * sizeof(char));
            sprintf(tmp, "%f", a->value.real);
            return tmp;
        }
        case AT_NULL:
            return strdup("null");
        case AT_BOOLEAN:
            return a->value.boolean ? strdup("true") : strdup("false");
        case AT_SYMBOL:
            return a->value.symbol ? strdup(a->value.symbol->name) : NULL;
        case AT_TEMPORARY: {
            return a->value.symbol ? strdup(a->value.symbol->name) : NULL;
        }
        case AT_LABEL: {
            char* tmp = malloc(10 * sizeof(char));
            sprintf(tmp, "L%d", a->value.label);
            return tmp;
        }
        default:
            return NULL;
    }
}

bool is_same_address(ADDRESS *a, ADDRESS *b){
	if(a == NULL && b!=NULL)
		return false;
	if(a!=NULL && b == NULL)
		return false;
	if(a==b)
		return true;
	if(a->meta != b->meta)
		return false;
	if(a->meta == AT_SYMBOL){
		return (a->value.symbol == b->value.symbol);
		}
	if(a->meta == AT_INT){
		return (a->value.integer == b->value.integer);
		}
	if(a->meta == AT_REAL){
		return (a->value.real == b->value.real);
		}
	return false;
}
