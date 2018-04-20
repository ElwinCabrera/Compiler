#ifndef ADDRESS_H
#define ADDRESS_H

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
        char boolean;
        double real;
        char character;
        char* string;
        struct symtab* symbol;
        int temporary;
        int null;
        int label;
        struct assignable* array;
    } value;
} ADDRESS;

ADDRESS* label_address(int);
ADDRESS* temp_address(struct symtype*);
ADDRESS* symbol_address(struct symtab*);
ADDRESS* int_address(int);
ADDRESS* boolean_address(int);
ADDRESS* real_address(double);
ADDRESS* char_address(char);
ADDRESS* string_address(char*);
ADDRESS* null_address();
int get_width(ADDRESS*);
char* create_address_string(ADDRESS*);

#endif
