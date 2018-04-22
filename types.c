#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "intermediate_code.h"
#include "symbol_table.h"
#include "types.h"
#include "expression.h"
#include "linked_list.h"

static TYPE_CONTAINER* known_types;

TYPE_CONTAINER* get_type_container() {
    if(!known_types) {
        known_types = malloc(sizeof(TYPE_CONTAINER));
    } 

    return known_types;
}

SYMTYPE* add_type(TYPE_CONTAINER* container, TTYPE meta, char* name, int width) {

    if(!name || find_type(container, name)) {
        return NULL;
    }

    SYMTYPE* new = malloc(sizeof(SYMTYPE));

    new->meta = meta;
    new->name = name;
    new->width = width;
    container->head = ll_insertfront(container->head, new);

    return new;
}

bool name_match_type(LINKED_LIST* l, void* name) {
  SYMTYPE* t = ll_value(l);
  return t && t->name && strcmp(t->name, name) == 0;
}

SYMTYPE* find_type(TYPE_CONTAINER* container, char* name) {

    if(!container) {
        return NULL;
    }

    return ll_find(container->head, name, name_match_type);
}

bool check_metatype(SYMTYPE* t, TTYPE meta) {

    if (!t) {
        return false;
    }

    return t->meta == meta;
}

bool check_typename(SYMTYPE* t, char* name) {

    if (!t || !name) {
        return false;
    }

    return strcmp(name, t->name) == 0;
}

bool compare_typenames(char* t1, char* t2) {
    if(!t1 || !t2) {
        return false;
    }

    return strcmp(t1, t2) == 0;
}

SYMTYPE* lval_type(TAC_OP op, SYMTYPE* lhs, SYMTYPE* rhs) {
    switch(op) {
        case I_IS_NULL:
        case I_NOT:
        case I_AND:
        case I_OR:
        case I_LESS_THAN:
        case I_EQUAL:
            return find_type(known_types, "Boolean");
        case I_REAL2INT:
        case I_MODULUS:
            return find_type(known_types, "integer");
        case I_INT2REAL:
            return find_type(known_types, "real");
        case I_MULTIPLY:
        case I_DIVIDE:
        case I_ADD:
        case I_SUB:
            return lhs;
        case I_ARRAY_ACCESS:
            return lhs->element_type;
        case I_CALL:
            return lhs->ret->type;
        default:
            return NULL;
    }
}

int get_type_width(SYMTYPE* t) {
    if(!t) {
        return 0;
    }

    return t->width;
}