#ifndef GRAPH_H
#define GRAPH_H

#include "linked_list.h"
#include "address.h"
#include "intermediate_code.h"


typedef struct valnum {
	TAC_OP op;
	int number;
	TAC *code;
	struct valnum* left;
	struct valnum* right;
	ADDRESS * result;
} VALNUM;


typedef struct graph{
	LINKED_LIST *head;//contain valnum
	VALNUM *current;
}GRAPH;


VALNUM* newNode(TAC_OP ,int, TAC *, VALNUM *, VALNUM *, ADDRESS *);
GRAPH* newGraph();
VALNUM *find_node_with_expression(GRAPH *,TAC_OP, VALNUM *, VALNUM *);
VALNUM *find_node_with_address(GRAPH *,ADDRESS *);
void process_tac(GRAPH *, TAC *);
void add_to_graph(GRAPH *,VALNUM *);
void print_graph(GRAPH*);
void optimize_common_exp(GRAPH *);
void optimize_dead_code(GRAPH *);
bool is_live_node(VALNUM *);
#endif
