#include <stdlib.h>
#include <string.h>
#include "address.h"

static int temporary_count = 0;

ADDRESS* new_address() {
    return malloc(sizeof(ADDRESS));
};

ADDRESS* temp_address(SYMTYPE* t) {
    ADDRESS* a = new_address();
    a->meta = TEMPORARY;
    a->value.temporary = temporary_count++;
    a->type = t;
    return a;
}

ADDRESS* symbol_address(SYMTAB* s) {
    ADDRESS* a = new_address();
    a->meta = SYMBOL;
    a->value.symbol = s;
    a->type = s ? s->type : NULL;
    return a;
}

ADDRESS* label_address(int n) {
    ADDRESS* a = new_address();
    a->meta = LABEL;
    a->value.label = n;
    a->type = NULL;
    return a;
}

ADDRESS* int_address(int n) {
    ADDRESS* a = new_address();
    a->meta = INT_CONSTANT;
    a->value.integer = n;
    a->type = find_type(get_type_container(), "integer");
    return a;
}

ADDRESS* boolean_address(int b) {
    ADDRESS* a = new_address();
    a->meta = BOOLEAN_CONSTANT;
    a->value.boolean = b;
    a->type = find_type(get_type_container(), "Boolean");
    return a;
}

ADDRESS* real_address(double d) {
    ADDRESS* a = new_address();
    a->meta = REAL_CONSTANT;
    a->value.real = d;
    a->type = find_type(get_type_container(), "real");
    return a;
};

ADDRESS* char_address(char c) {
    ADDRESS* a = new_address();
    a->meta = CHAR_CONSTANT;
    a->value.character = c;
    a->type = find_type(get_type_container(), "character");
    return a;
};

ADDRESS* string_address(char* s) {
    ADDRESS* a = new_address();
    a->meta = STRING_CONSTANT;
    a->value.string = s;
    a->type = find_type(get_type_container(), "string");
    return a;
};

ADDRESS* null_address() {
    ADDRESS* a = new_address();
    a->value.null = 0;
    a->meta = NULL_CONSTANT;
    a->type = find_type(get_type_container(), "nullconst");
    return a;
}

char* create_address_string(ADDRESS* a) {
    if(!a) {
        return NULL;
    }

    switch(a->meta) {
        case STRING_CONSTANT:
            return strdup(a->value.string);
        case CHAR_CONSTANT: {
            char * tmp = malloc(2 * sizeof(char));
            sprintf(tmp, "%c", a->value.character);
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