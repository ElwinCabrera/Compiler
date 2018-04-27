#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "data_block.h"
#include "linked_list.h"
#include "address.h"

static DATA_BLOCK* _data = NULL;

int string_count = 0;
int real_count = 0;

DATA_BLOCK* get_data_block() {
    if(!_data) {
        _data = malloc(sizeof(DATA_BLOCK));
        memset(_data, 0, sizeof(DATA_BLOCK));
    }
    return _data;
}

bool match_const(LINKED_LIST* l, void* match) {
    DATA* a = ll_value(l);
    ADDRESS* cmp = (ADDRESS*) match;
    switch(a->type) {
        case AT_REAL:
            return cmp->meta == AT_REAL && cmp->value.real == a->constant->value.real;
        case AT_STRING:
            return cmp->meta == AT_STRING && strcmp(cmp->value.string, a->constant->value.string) == 0;
        default:
            return false;
    }
}

void print_data_block(FILE* f) {
    
    if(!f) {
        f = stdout;
    }
    
    DATA_BLOCK* db = get_data_block();
    
    LINKED_LIST* list = db->constants;
    ll_reverse(&list);

    fprintf(f, "// DATA\n\n");
    while(list) {
        DATA* d = ll_value(list);
        switch(d->type) {
            case AT_REAL:
                fprintf(f, "%s REAL %f\n", d->name, d->constant->value.real);
                break;
            case AT_STRING:
                fprintf(f, "%s STRING %s\n", d->name, d->constant->value.string);
                break;
            default:
                break;
        }
        list = ll_next(list);
    }

    fprintf(f, "\n// END DATA\n\n");
}

DATA* new_real_data(ADDRESS* a){
    DATA* d = malloc(sizeof(DATA));
    d->type = AT_REAL;
    d->constant = a;
    d->name = malloc(12);
    sprintf(d->name, "real_%02d", real_count++);
    return d;
}

DATA* new_string_data(ADDRESS* a){
    DATA* d = malloc(sizeof(DATA));
    d->type = AT_STRING;
    d->constant = a;
    d->name = malloc(12);
    sprintf(d->name, "str_%02d", string_count++);
    return d;
}

DATA* store_in_data_block(ADDRESS* a) {

    DATA_BLOCK* db = get_data_block();
    
    LINKED_LIST* constants = db->constants;

    DATA* d = ll_find(constants, a, match_const);

    if(!d) {
        switch(a->meta) {
            case AT_REAL:
                d = new_real_data(a);
                break;
            case AT_STRING:
                d = new_string_data(a);
                break;
            default:
                printf("Improperly storing data in data block\n");
                break;
        }
    }

    db->constants = ll_insertfront(db->constants, d);

    return d;
}