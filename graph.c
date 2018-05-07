#include "address.h"
#include "symbol_table.h"
#include "types.h"
#include "assignable.h"
#include "linked_list.h"
#include "code_block.h"
#include "intermediate_code.h"


VALNUM* newNode(TAC_OP OP,int number, TAC *instruction, VALNUM *left, VALNUM *right, VALNUM *value){
	VALNUM *node = malloc(sizeof(VALNUM));
	node->op = OP;
	node->number = number;
	node->instruction = instruction;
	node->left = left;
	node->right = right;
	node->value = value;
	return node;
};

GRAPH* newGraph(){
	GRAPH *gr = malloc(sizeof(GRAPH));
	gr->head = NULL;
	gr->current = NULL;
	gr->ll = NULL;
	return gr;
}

void add_to_graph(GRAPH *gr,VALNUM *node){
	if(gr->head==NULL){
		gr->head = ll_insertfront(gr->head,node);
		gr->current = node;
	}
	else{
		gr->head = ll_insertback(gr->head,node);
		gr->current = node;
	}
}

void process_tac(GRAPH *gr, TAC *code){
	// process the three address code
	//first step is to analyze the three address code
	TAC_OP OP = code->op;
};

VALNUM *find_node_with_address(GRAPH *gr,ADDRESS *add){
	VALNUM *result = NULL;
	LINKED_LIST *head = gr->head;
	while(head!=NULL){
		if(head->value->value == add){
			result = head->value;
		} 
	}
	return result;
};

VALNUM *find_node_with_expression(TAC_OP op, VALNUM *left, VALNUM *right){
	LINKED_LIST *head = gr->head;
	while(head!=NULL){
		VALNUM *node = head->value;
		TAC_OP cur = node->op;
		if(node->op == cur && node-left == left && node->right == right){
			return node;
		}
	}
	return NULL;
};
