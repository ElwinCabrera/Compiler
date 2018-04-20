#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdbool.h>

/*
    Linked list:

    ll_new(p) : creates a new linked list with generic pointer type p
    ll_hasnext(l) : checks if l-> next is null
    ll_find(l, match, f) : iterates given linked list and applies
    f(l, match). Retursn the value of the matched node if true (generic pointer)
    ll_combine(l1, l2) : attaches the head of one list to the tail of the next and
        returns the new head. l1 will always be the new head node if ll_hasnext(l2), 
        otherwise l2 will be the new head node
    ll_insertfront(l,v) : creates a new node with v, attaches l to next of the new node,
        returns the new node
    ll_insertback(l,v) : creates a new node with v, attaches recursively calls ll_next(l) until
        ll_hasnext(l) is false, and attaches the node to the head of that node
        returns l for completeness
    ll_next(l) : returns l->next. protects against bad dereferences
    ll_value(l) : returns the node held in l
    ll_mergesort(&l, c) : merge sorts a linked list by updating references ONLY. takes the ref to
        linked list and updates the value of that reference to the head of a linked list sorted by
        a comparator function c
*/
typedef struct linked_list {
    struct linked_list* next;
    void* value;
} LINKED_LIST;

void* ll_find(LINKED_LIST*, void*, bool(LINKED_LIST*, void*));
bool ll_hasnext(LINKED_LIST* l);
LINKED_LIST* ll_new(void*);
LINKED_LIST* ll_combine(LINKED_LIST*, LINKED_LIST*);
LINKED_LIST* ll_insertfront(LINKED_LIST*, void*);
LINKED_LIST* ll_insertback(LINKED_LIST*, void*);
LINKED_LIST* ll_next(LINKED_LIST*);
void* ll_value(LINKED_LIST*);
void ll_mergesort(LINKED_LIST**, void*);

#endif