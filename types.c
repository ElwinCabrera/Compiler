#include <stdlib.h>
#include <string.h>
#include "types.h"

SYMTYPE * add_type(SYMTYPE* start, TYPEMETA meta, char* name) {

    if(!name || find_type(start, name)) {
        return NULL;
    }

    SYMTYPE * new = malloc(sizeof(SYMTYPE));

    switch(meta) {
        case MT_FUNCTION:
            new->details.function = malloc(sizeof(struct function_details));
            break;
        case MT_RECORD:
            new->details.record = malloc(sizeof(struct record_details));
            break;
        case MT_ARRAY:
            new->details.array = malloc(sizeof(struct array_details));
            break;
        default:
            new->details.primitive = NULL;
            break;
    }

    new->meta = meta;
    new->name = name;
    new->next = start;

    return new;
}

SYMTYPE * find_type(SYMTYPE* start, char* name) {

    if(!name) {
        return NULL;
    }

    SYMTYPE * s = start;

    while(s) {
        if(strcmp(name, s->name) == 0) {
            return s;
        }
        s = s->next;
    }

    return NULL;
}

int check_metatype(SYMTYPE* t, TYPEMETA meta) {

    if (!t) {
        return 0;
    }

    return t->meta == meta;
}

int check_type(SYMTYPE* t, char* name) {

    if (!t || !name) {
        return 0;
    }

    if (strcmp(name, t->name) == 0) {
        return 1;
    }

    return 0;
}