#ifndef GRAPH_H
#define GRAPH_H

#include "address.h"
#include "symbol_table.h"
#include "types.h"
#include "assignable.h"
#include "linked_list.h"
#include "code_block.h"
#include "intermediate_code.h"


typedef struct valnum {
	TAC_OP op;
	int number;
	TAC *instruction;
	VALNUM* left;
	VALNUM* right;
	VALNUM* value;
} VALNUM;


typedef struct graph{
	LINKED_LIST *head;//contain valnum
	VALNUM *current;
}GRAPH;


VALNUM* newNode(TAC_OP ,int number, TAC *, VALNUM *, VALNUM *, ADDRESS *);
GRAPH* newGraph();


#endif
