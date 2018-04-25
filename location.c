#include <stdlib.h>
#include <string.h>
#include "location.h"

LOCATION* new_location() {
    LOCATION* l = malloc(sizeof(LOCATION));
    memset(l, 0, sizeof(LOCATION));
    return l;
}

LOCATION* register_location(REG r) {
    LOCATION* l = new_location();
    l->type = W_REGISTER;
    l->value.reg = r;
    return l;
}

LOCATION* data_location(DATA* d) {
    LOCATION* l = new_location();
    l->type = W_BLOCK;
    l->value.data = d;
    return l;
}

LOCATION* memory_location(int i) {
    LOCATION* l = new_location();
    l->type = W_MEMORY;
    l->value.pointer = i;
    return l;
}

LOCATION* const_location(int i) {
    LOCATION* l = new_location();
    l->type = W_CONSTANT;
    l->value.constant = i;
    return l;
}

char* create_location_str(LOCATION* l) {
    if(!l) {
        return NULL;
    }

    char* str;

    switch(l->type) {
        case W_REGISTER:
            str = malloc(8 * sizeof(char));
            sprintf(str, "R%d", l->value.reg);
            break;
        case W_MEMORY:
            str = malloc(12 * sizeof(char));
            sprintf(str, "%04x", l->value.pointer);
            break;
        case W_CONSTANT:
            str = malloc(12 * sizeof(char));
            sprintf(str, "#%d", l->value.constant);
            break;
        case W_BLOCK:
            str = strdup(l->value.data->name);
            break;
    }

    return str;
}