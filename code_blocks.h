#ifndef CODE_BLOCKS_H
#define CODE_BLOCKS_H

#include <stdio.h>
#include "intermediate_code.h"
#include "graph.h"

typedef struct block {
    int label;
    struct linked_list* code;
    struct linked_list* edges;
    struct linked_list* live_on_exit;
    struct linked_list* vars;
	GRAPH *optimizer;
} BLOCK;

void print_blocks(struct linked_list*, FILE*);
BLOCK* new_code_block(int);
void mark_leaders(struct intermediate_code*);
struct linked_list* make_blocks(struct intermediate_code*);
GRAPH *create_optimize_graph(BLOCK *);
void common_expression_optimize(LINKED_LIST *);
void dead_code_optimize(LINKED_LIST *);
#endif
