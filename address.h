#ifndef ADDRESS_H
#define ADDRESS_H

#include "types.h"
#include "symbol_table.h"

typedef enum atype {
    SYMBOL,
    INT_CONSTANT,
    REAL_CONSTANT,
    BOOLEAN_CONSTANT,
    CHAR_CONSTANT,
    STRING_CONSTANT,
    NULL_CONSTANT,
    TEMPORARY,
    LABEL,
} ATYPE;

typedef struct address {
    ATYPE meta;
    struct symtype* type;
    union {
        int integer;
        int boolean;
        double real;
        char character;
        char* string;
        struct symtab* symbol;
        int temporary;
        int null;
        int label;
    } value;
} ADDRESS;

ADDRESS* label_address(int);
ADDRESS* temp_address(SYMTYPE*);
ADDRESS* symbol_address(SYMTAB*);
ADDRESS* int_address(int);
ADDRESS* boolean_address(int);
ADDRESS* real_address(double);
ADDRESS* char_address(char);
ADDRESS* string_address(char*);
ADDRESS* null_address();
char* create_address_string(ADDRESS*);

#endif
