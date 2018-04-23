#ifndef ADDRESS_H
#define ADDRESS_H

#include <stdbool.h>

typedef enum atype {
    AT_SYMBOL,
    AT_INT,
    AT_REAL,
    AT_BOOLEAN,
    AT_CHAR,
    AT_STRING,
    AT_NULL,
    AT_TEMPORARY,
    AT_LABEL,
} ATYPE;

typedef struct address {
    ATYPE meta;
    struct data* block;
    struct symtype* type;
    union {
        int integer;
        char boolean;
        double real;
        char character;
        char* string;
        struct symbol* symbol;
        int null;
        int label;
        struct assignable* array;
    } value;
} ADDRESS;

ADDRESS* label_address(int);
ADDRESS* temp_address(struct symtype*);
ADDRESS* symbol_address(struct symbol*);
ADDRESS* int_address(int);
ADDRESS* boolean_address(int);
ADDRESS* real_address(double);
ADDRESS* char_address(char);
ADDRESS* string_address(char*);
ADDRESS* null_address();
int get_width(ADDRESS*);
char* create_address_string(ADDRESS*);

#endif
