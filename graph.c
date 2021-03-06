#include<stdlib.h>

#include "graph.h"
#include "address.h"
#include "symbol_table.h"
#include "types.h"
#include "assignable.h"
#include "linked_list.h"
#include "code_blocks.h"
#include "intermediate_code.h"
#include <stdbool.h>

VALNUM* newNode(TAC_OP OP,int number, TAC *code, VALNUM *left, VALNUM *right, ADDRESS *value){
	VALNUM *node = malloc(sizeof(VALNUM));
	node->op = OP;
	node->number = number;
	node->code = code;
	node->left = left;
	node->right = right;
	node->result = value;
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
			
		printf("%d %d %d\n",node->number, node->left ? node->left->number : -1 , node->right ? node->right->number : -1);

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
		gr->head = ll_insertback(gr->head,node);
		gr->current = node;
	}
}

void process_tac(GRAPH *gr, TAC *code){
	// process the three address code
	//first step is to analyze the three address code
	TAC_OP OP = code->op;
	ADDRESS *left = code->x;
	ADDRESS *right = code->y;
	ADDRESS *result  = code->result;
	//need to check if left and right are already in the graph
	VALNUM *ll = find_node_with_address(gr,left);
	if (ll == NULL){
		// create a new ll node
		int number = 0;
		if (gr->current==NULL)
			number = 0;
		else 
			number = gr->current->number+1;
		ll = newNode(I_VALNUM_NODE,number,NULL,NULL,NULL,left);
		add_to_graph(gr,ll);
	}
	VALNUM *rr = find_node_with_address(gr,right);
	if (rr == NULL && right !=NULL){
		// create a new rr node
		rr = newNode(I_VALNUM_NODE,gr->current->number + 1,NULL,NULL,NULL,right);
		add_to_graph(gr,rr);
	}
	VALNUM *old_node = find_node_with_expression(gr,OP,ll,rr);
	if(old_node==NULL || old_node->number < ll->number || old_node->number < rr->number){
		// need to make a new node for this expression
		VALNUM *node = newNode(OP,gr->current->number+1,code,ll,rr,result);
		add_to_graph(gr,node);	
	}
	else{
		//node with that expression already existed
		//just need to assign to that node
		VALNUM *node = newNode(I_VALNUM_LINKED,gr->current->number+1,code,old_node,NULL,result);
		add_to_graph(gr,node);
	}
	
}

void optimize_common_exp(GRAPH *gr){
	// optimization, changing all the code
	LINKED_LIST *head = gr->head;
	while(head){
		//change the tac
		VALNUM *node = head->value;
		if(node->code!=NULL){
			if(node->op == I_VALNUM_LINKED){
				TAC *code = node->code;
				code->op = I_ASSIGN;
				code->y = NULL;
				code->x = node->left->result;		
			}
			else{
				TAC *code = node->code;
				code->x = node->left->result;
				if(node->right)
					code->y = node->right->result;
			}
		}	
		head = ll_next(head);

	}

}

bool is_live_node(VALNUM *node){
	if(!node->code)
		return false;
	return node->code->x_live || node->code->y_live || node->code->result_live;
}

void optimize_dead_code(GRAPH *gr){
	// need to check every root nodes
	// if the two children are live, else delete
	int parent[gr->current->number+1];
	//find the parents of all node
	LINKED_LIST *head = gr->head;
	while(head){
		VALNUM *node = head->value;
		parent[node->number] = -1;
		if(node->left)
			parent[node->left->number] = node->number;
		if(node->right)
			parent[node->right->number] = node->number;
		head = ll_next(head);
	}
	bool changed = true;
	while(changed){
		changed = false;
		head = gr->head;
		while(head){
			VALNUM *node = head->value;
			if(parent[node->number] == -1 && is_live_node(node) == false){
				if (node->code)
					node->code->op = I_NOP;
				parent[node->number] = -2;
				if(node->left)
					parent[node->left->number] = -1;
				if(node->right)
					parent[node->right->number] = -1;
				changed = true;
			}
			head = ll_next(head);
		}
	}
}


VALNUM *find_node_with_address(GRAPH *gr,ADDRESS *add){
	VALNUM *result = NULL;
	LINKED_LIST *head = gr->head;
	while(head!=NULL){
		VALNUM *node = head->value;
		ADDRESS *cur_add = node->result;
		if(is_same_address(cur_add,add)){
			result = node;
		} 
		head = ll_next(head);
	}
	return result;
}

VALNUM *find_node_with_expression(GRAPH *gr,TAC_OP op, VALNUM *left, VALNUM *right){
	LINKED_LIST *head = gr->head;
	while(head!=NULL){
		VALNUM *node = head->value;
		if(node->op == op && node->left == left && node->right == right){
			return node;
		}
		if(op == I_MULTIPLY || op == I_ADD){
			//account for commutativity for + *
			if(node->left == right && node->right == left){
				return node;
			}		
		}
		head = ll_next(head);
	}
	return NULL;
}
