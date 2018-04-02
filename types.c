#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "y.tab.h"

SYMTYPE * add_type(SYMTYPE* start, int type, char* name) {

    if(!name || find_type(start, name)) {
        return NULL;
    }

    SYMTYPE * new = malloc(sizeof(SYMTYPE));
    
    new->subtype = type;

    switch(type) {
        case FUNCTION:
            new->details.record = malloc(sizeof(struct function_details));
            break;
        case RECORD:
            new->details.record = malloc(sizeof(struct record_details));
            break;
        case ARRAY:
            new->details.array = malloc(sizeof(struct array_details));
            break;
        default:
            new->details.primitive = NULL;
            break;
    }
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

int check_type(SYMTYPE* t, int subtype, char* name) {

    if (!t) {
        return 0;
    }

    if(t->subtype == subtype) {

        if (!name || strcmp(name, t->name) == 0) {
            return 1;
        }

        return 0;
    }

    return 0;
}