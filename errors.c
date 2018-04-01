
#include <stdlib.h>
#include <string.h>
#include "errors.h"

ERROR * push_error(ERROR* e, char* c) {
    ERROR * new = malloc(sizeof(ERROR));
    new->e = strdup(c);
    new->next = e;
    return new;
}

ERROR * pop_error(ERROR* e) {
    if(!e) {
        return NULL;
    }

    return e->next;
}

void free_error(ERROR* e) {
    if(!e) {
        return;
    }

    if(e->e) {
        free(e->e);
    }

    free(e);
}