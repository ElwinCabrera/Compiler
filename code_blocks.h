#ifndef CODE_BLOCKS_H
#define CODE_BLOCKS_H

#include <stdio.h>
#include "intermediate_code.h"

typedef struct block {
    int label;
    struct linked_list* code;
    struct linked_list* edges;
    struct linked_list* live_on_exit;
    struct linked_list* vars;
} BLOCK;

void print_blocks(struct linked_list*, FILE*);
BLOCK* new_code_block(int);
void mark_leaders(struct intermediate_code*);
struct linked_list* make_blocks(struct intermediate_code*);

#endif