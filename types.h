#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include "intermediate_code.h"

typedef enum ttype {
    MT_FUNCTION,
    MT_ARRAY,
    MT_RECORD,
    MT_PRIMITIVE,
} TTYPE;

typedef struct symtype {
    TTYPE meta;
    char* name;
    struct scope* parameters;
    struct symtab* ret;
    struct scope* members;
    struct symtype* element_type;
    int dimensions;
    int width;
    struct symtype* next;
} SYMTYPE;

typedef struct type_container {
    struct linked_list* head;
} TYPE_CONTAINER;


TYPE_CONTAINER* get_type_container();
SYMTYPE* add_type(TYPE_CONTAINER*, TTYPE, char*, int);
SYMTYPE* find_type(TYPE_CONTAINER*, char*);
SYMTYPE* lval_type(TAC_OP, SYMTYPE*, SYMTYPE*);
bool check_metatype(SYMTYPE*, TTYPE);
bool check_typename(SYMTYPE*, char*);
bool compare_typenames(char*, char*);
int get_type_width(SYMTYPE*);

#endif