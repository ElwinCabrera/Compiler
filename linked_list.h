#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdbool.h>

typedef struct linked_list {
    struct linked_list* next;
    void* value;
} LINKED_LIST;

void* ll_find(LINKED_LIST*, void*, bool(LINKED_LIST*, void*));
bool ll_hasnext(LINKED_LIST* l);
LINKED_LIST* ll_new(void*);
LINKED_LIST* ll_combine(LINKED_LIST*, LINKED_LIST*);
LINKED_LIST* ll_search(LINKED_LIST*, void*);
LINKED_LIST* ll_insertfront(LINKED_LIST*, void*);
LINKED_LIST* ll_insertback(LINKED_LIST*, void*);
LINKED_LIST* ll_next(LINKED_LIST*);
void* ll_value(LINKED_LIST*);
void ll_mergesort(LINKED_LIST**, void*);

#endif