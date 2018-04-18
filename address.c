#include <stdlib.h>
#include <string.h>
#include "address.h"

static int temporary_count = 0;

/*
    According to text, an address can be:
        - A symbol (name)
        - A constant
        - A temporary

    This is a generic wrapper for an address creation
*/
ADDRESS* new_address() {
    return malloc(sizeof(ADDRESS));
};

/*
    Creates an address for a temporary value.
    Increases the temporary variable counter.
*/
ADDRESS* temp_address(SYMTYPE* t) {
    ADDRESS* a = new_address();
    a->meta = TEMPORARY;
    a->value.temporary = temporary_count++;
    a->type = t;
    return a;
}

/*
    Creates an address from a symbol table entry
*/
ADDRESS* symbol_address(SYMTAB* s) {
    ADDRESS* a = new_address();
    a->meta = SYMBOL;
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
    a->meta = LABEL;
    a->value.label = n;
    a->type = NULL;
    return a;
}

/*
    integer constant
*/
ADDRESS* int_address(int n) {
    ADDRESS* a = new_address();
    a->meta = INT_CONSTANT;
    a->value.integer = n;
    a->type = find_type(get_type_container(), "integer");
    return a;
}

/*
    boolean constant
*/
ADDRESS* boolean_address(int b) {
    ADDRESS* a = new_address();
    a->meta = BOOLEAN_CONSTANT;
    a->value.boolean = b;
    a->type = find_type(get_type_container(), "Boolean");
    return a;
}

/*
    real constant
*/
ADDRESS* real_address(double d) {
    ADDRESS* a = new_address();
    a->meta = REAL_CONSTANT;
    a->value.real = d;
    a->type = find_type(get_type_container(), "real");
    return a;
};

/*
    char constant
*/
ADDRESS* char_address(char c) {
    ADDRESS* a = new_address();
    a->meta = CHAR_CONSTANT;
    a->value.character = c;
    a->type = find_type(get_type_container(), "character");
    return a;
};

/*
    string constant
*/
ADDRESS* string_address(char* s) {
    ADDRESS* a = new_address();
    a->meta = STRING_CONSTANT;
    a->value.string = s;
    a->type = find_type(get_type_container(), "string");
    return a;
};

/*
    null constant
*/
ADDRESS* null_address() {
    ADDRESS* a = new_address();
    a->value.null = 0;
    a->meta = NULL_CONSTANT;
    a->type = find_type(get_type_container(), "nullconst");
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
        case STRING_CONSTANT:
            return strdup(a->value.string);
        case CHAR_CONSTANT: {
            char * tmp = malloc(2 * sizeof(char));
            sprintf(tmp, "'%c'", a->value.character);
            return tmp;
        }
        case INT_CONSTANT: {
            char * tmp = malloc(20 * sizeof(char));
            sprintf(tmp, "%d", a->value.integer);
            return tmp;
        }
        case REAL_CONSTANT: {
            char * tmp = malloc(20 * sizeof(char));
            sprintf(tmp, "%f", a->value.real);
            return tmp;
        }
        case NULL_CONSTANT:
            return strdup("null");
        case BOOLEAN_CONSTANT:
            return a->value.boolean ? strdup("true") : strdup("false");
        case SYMBOL:
            return a->value.symbol ? strdup(a->value.symbol->name) : NULL;
        case TEMPORARY: {
            char * tmp = malloc(10 * sizeof(char));
            sprintf(tmp, "t%d", a->value.temporary);
            return tmp;
        }
        case LABEL: {
            char * tmp = malloc(10 * sizeof(char));
            sprintf(tmp, "L%d", a->value.label);
            return tmp;
        }
        default:
            return NULL;
    }
}