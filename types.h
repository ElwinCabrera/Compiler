#ifndef TYPES_H
#define TYPES_H

struct function_details {
    struct scope * parameters;
    struct type * return_type;
};

struct array_details {
    int dimensions;
    struct type * element_type;
};

struct record_details {
    struct scope * members;
};

typedef struct type {
    int subtype;
    char * name;
    union {
        struct function_details * function;
        struct array_details * array;
        struct record_details * record;
        void * primitive;
    } details;
    struct type * next;
} SYMTYPE;

SYMTYPE* type_list;

SYMTYPE* add_type(SYMTYPE*, int, char*);
SYMTYPE* find_type(SYMTYPE*, char*);
int check_type(SYMTYPE*,int,char*);

#endif