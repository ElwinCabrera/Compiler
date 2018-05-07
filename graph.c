#include<stdlib.h>

#include "graph.h"
#include "address.h"
#include "symbol_table.h"
#include "types.h"
#include "assignable.h"
#include "linked_list.h"
#include "code_blocks.h"
#include "intermediate_code.h"


VALNUM* newNode(TAC_OP OP,int number, TAC *instruction, VALNUM *left, VALNUM *right, ADDRESS *value){
	VALNUM *node = malloc(sizeof(VALNUM));
	node->op = OP;
	node->number = number;
	node->instruction = instruction;
	node->left = left;
	node->right = right;
	node->value = value;
	return node;
}

GRAPH* newGraph(){
	GRAPH *gr = malloc(sizeof(GRAPH));
	gr->head = NULL;
	gr->current = NULL;
	return gr;
}

void print_graph(GRAPH *gr){
	LINKED_LIST *head = gr->head;
	while(head){
		VALNUM *node = head->value;
		printf("%d\n",node->number);
		head = ll_next(head);	
	}
	printf("DONE GRAPH\n");
}


void add_to_graph(GRAPH *gr,VALNUM *node){
	if(gr->head==NULL){
		gr->head = ll_new(node);
		gr->current = node;
	}
	else{
		printf("ADDING TO GRAPH ERROR FUNCTION\n");
		gr->head = ll_insertback(gr->head,node);
		printf("ADDING TO GRAPH ERROR FUNCTION2\n");
		gr->current = node;
	}
}

void process_tac(GRAPH *gr, TAC *code){
	// process the three address code
	//first step is to analyze the three address code
	TAC_OP OP = code->op;
	VALNUM *node = newNode(OP,0,code,NULL,NULL,NULL);
	if(gr->head!=NULL){
		node->number = gr->current->number + 1;	
	}
	printf("HERE STUCK\n");
	add_to_graph(gr,node);
	printf("DONE PROCESS TAC INNER\n");
}

VALNUM *find_node_with_address(GRAPH *gr,ADDRESS *add){
	VALNUM *result = NULL;
	LINKED_LIST *head = gr->head;
	while(head!=NULL){
		VALNUM *node = head->value;
		ADDRESS *cur_add = node->value;
		if(cur_add == add){
			result = head->value;
		} 
		head = ll_next(head);
	}
	return result;
}

VALNUM *find_node_with_expression(GRAPH *gr,TAC_OP op, VALNUM *left, VALNUM *right){
	LINKED_LIST *head = gr->head;
	while(head!=NULL){
		VALNUM *node = head->value;
		TAC_OP cur = node->op;
		if(node->op == cur && node->left == left && node->right == right){
			return node;
		}
		head = ll_next(head);
	}
	return NULL;
}
