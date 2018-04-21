#ifndef CODE_BLOCKS_H
#define CODE_BLOCKS_H

#include "intermediate_code.h"

typedef struct block {
    int label;
    struct linked_list* code;
} BLOCK;

void mark_leaders(struct intermediate_code*);
struct linked_list* make_blocks(struct intermediate_code*);

#endif