#ifndef NODE_H
#define NODE_H

#include <stdlib.h>

typedef enum nodetype {
    INT_CONSTANT,
    REAL_CONSTANT,
    BOOLEAN_CONSTANT,
    CHAR_CONSTANT,
    STRING_CONSTANT,
    NULL_CONSTANT,
    SYMBOL,
    CODE,
} NODETYPE;

typedef struct node {
    int id;
    char* type_name;
    NODETYPE meta;
    union {
        int integer;
        int boolean;
        double real;
        char character;
        char* string;
        struct symtab* symbol;
        struct ir* instruction;
    } value;
} NODE;


NODE* int_node(int);
NODE* boolean_node(int);
NODE* real_node(double);
NODE* char_node(char);
NODE* string_node(char*);
NODE* null_node();
NODE* symbol_node(struct symtab*);
NODE* ir_node(struct ir*,char*);

#endif