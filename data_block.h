#ifndef DATA_BLOCK_H
#define DATA_BLOCK_H

#include <stdio.h>
#include "linked_list.h"
#include "address.h"

typedef struct data {
    ATYPE type;
    ADDRESS* constant;
    char* name;
} DATA;

typedef struct data_block {
    LINKED_LIST* constants;
} DATA_BLOCK;

DATA_BLOCK* get_data_block();
void print_data_block(FILE*);
DATA* store_in_data_block(struct address*);

#endif