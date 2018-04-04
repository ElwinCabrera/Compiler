#ifndef TYPES_H
#define TYPES_H

struct function_details {
    struct scope * parameters;
    struct symtype * return_type;
};

struct array_details {
    int dimensions;
    struct symtype * element_type;
};

struct record_details {
    struct scope * members;
};

typedef struct symtype {
    int subtype;
    char * name;
    union {
        struct function_details * function;
        struct array_details * array;
        struct record_details * record;
        void * primitive;
    } details;
    struct symtype * next;
} SYMTYPE;

SYMTYPE* type_list;

SYMTYPE* add_type(SYMTYPE*, int, char*);
SYMTYPE* find_type(SYMTYPE*, char*);
int check_type(SYMTYPE*,int,char*);

#endif