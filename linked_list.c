#include <stdlib.h>
#include <stdbool.h>
#include "linked_list.h"

LINKED_LIST* ll_new(void* value) {
    LINKED_LIST* new = malloc(sizeof(LINKED_LIST));
    new->next = NULL;
    new->value = value;
    return new;
}

LINKED_LIST* ll_combine(LINKED_LIST* src, LINKED_LIST* add) {
    
    if(!src) {
        return add;
    }

    /*
        A lot of times we'll just be adding a singular node
        to a constructed list, so this takes advantage of that
    */
    if(!ll_hasnext(add)) {
        add->next = src;
        return add;
    }
   
    LINKED_LIST* end = src;
    while(ll_hasnext(end)) {
        end = ll_next(end);
    }
    
    end->next = add;

    return src;
}

LINKED_LIST* ll_append(LINKED_LIST* head, LINKED_LIST* node) {
    node->next = head;
    return node;
}

LINKED_LIST* ll_insertfront(LINKED_LIST* head, void* value) {
    LINKED_LIST* new = malloc(sizeof(LINKED_LIST));
    new->next = head;
    new->value = value;
    return new;
}

LINKED_LIST* ll_insertback(LINKED_LIST* head, void* value) {
    LINKED_LIST* new = malloc(sizeof(LINKED_LIST));
    new->value = value;

    if(!head) {
        return new;
    }
    LINKED_LIST* tail = head;
    while(ll_hasnext(tail)) {
        tail = ll_next(tail);
    }

    tail->next = new;
    
    return head;
}

bool ll_hasnext(LINKED_LIST* l) {
    return l && l->next ? true : false;
}

LINKED_LIST* ll_next(LINKED_LIST* l) {
    return l ? l->next : NULL;
}

void* ll_value(LINKED_LIST* l) {
    return l ? l->value : NULL;
}

LINKED_LIST* ll_merge(LINKED_LIST* lhs, LINKED_LIST* rhs, bool comparator(LINKED_LIST*, LINKED_LIST*)) {

    LINKED_LIST* result;
    
    if(rhs == NULL) return lhs;
    else if(lhs == NULL) return rhs;
    
    if(comparator(lhs, rhs)) {
        result = lhs;
        result->next = ll_merge(lhs->next, rhs, comparator);
    } else {
        result = rhs;
        result->next = ll_merge(lhs, rhs->next, comparator);
    }

    return result;
}

void ll_split(LINKED_LIST* list, LINKED_LIST** mid_point) {

    /*
        Basic idea is to have a fast advancer and slow advancer.
        Slow pace is 1, fast is 2. When fast sees a null node, slow
        is halfway through (because it advanced at half pace)

        We are guaranteed a first ->next from merge_sort's base case
    */


   LINKED_LIST* slow = list;
   LINKED_LIST* fast = list->next;

   while(fast) {
       fast = fast->next;
       if(fast) {
           slow = slow->next;
           fast = fast->next;
       }
   }

   (*mid_point) = slow->next;
   slow->next = NULL;
    
}

void ll_mergesort(LINKED_LIST** ref, void* comparator) {
    LINKED_LIST* lhs = *ref;

    if(!lhs || !lhs->next) {
        return;
    }

    LINKED_LIST* rhs;

    ll_split(lhs, &rhs);

    ll_mergesort(&lhs, comparator);
    ll_mergesort(&rhs, comparator);

    *ref = ll_merge(lhs, rhs, comparator);
}