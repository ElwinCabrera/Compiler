#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

typedef enum typemeta {
    MT_FUNCTION,
    MT_ARRAY,
    MT_RECORD,
    MT_PRIMITIVE,
} TYPEMETA;


typedef enum tc_result {
    FAIL,
    PASS,
    COERCE_LHS,
    COERCE_RHS,
} TC_RESULT;

struct function_details {
    struct scope * parameters;
    struct symtab * return_type;
};

struct array_details {
    int dimensions;
    struct symtype * element_type;
};

struct record_details {
    struct scope * members;
};

typedef struct symtype {
    TYPEMETA meta;
    char* name;
    union {
        struct function_details* function;
        struct array_details* array;
        struct record_details* record;
        void* primitive;
    } details;
    struct symtype* next;
} SYMTYPE;

SYMTYPE* type_list;

SYMTYPE* add_type(SYMTYPE*, TYPEMETA, char*);
SYMTYPE* find_type(SYMTYPE*, char*);
bool check_metatype(SYMTYPE*, TYPEMETA);
bool check_type(SYMTYPE*, char*);
bool compare_types(char*, char*);
TC_RESULT type_check_binary_expression(int, char*, char*);
TC_RESULT type_check_unary_expression(int, char*);

#endif