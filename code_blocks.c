#include <stdlib.h>
#include "code_blocks.h"
#include "linked_list.h"
#include "address.h"
#include "symbol_table.h"
#include "graph.h"

BLOCK* new_code_block(int i) {
    BLOCK* b = malloc(sizeof(BLOCK));
    b->code = NULL;
    b->live_on_exit = NULL;
    b->vars = NULL;
    b->edges = NULL;
    b->label = i;
    b->optimizer = newGraph();
    return b;
}

void print_blocks(LINKED_LIST* l, FILE* f) {

    while(l) {
        BLOCK* b = ll_value(l);
        fprintf(f ? f : stdout, "Block %d\n", b->label);
        LINKED_LIST* code_list = b->code;
	while(code_list) {
		TAC *code = ll_value(code_list);
		print_tac(code, f);	
		code_list = ll_next(code_list);
	}
        fprintf(f ? f : stdout, "\n");
        l = ll_next(l);
    }
}

void set_next_use_information(BLOCK* code_block) {
    if(!code_block) {
        return;
    }

    LINKED_LIST* statements = code_block->code;

    while(statements) {
        TAC* c = ll_value(statements);
        if(c) {

            bool result = c->result && (c->result->meta == AT_SYMBOL || c->result->meta == AT_TEMPORARY);
            bool x = c->x && (c->x->meta == AT_SYMBOL || c->x->meta == AT_TEMPORARY);
            bool y = c->y && (c->y->meta == AT_SYMBOL || c->y->meta == AT_TEMPORARY);

            /*
                1. Attach to statement i the information currently found in the symbol table
                regarding the next use and liveness of x, y, and y. 
            */

            if(result) {
                if(!ll_find(code_block->vars, c->result->value.symbol, pointer_match)) {
                    code_block->vars = ll_insertfront(code_block->vars, c->result->value.symbol);
                }

                c->result_live = c->result->value.symbol->live;
                c->result_next = c->result->value.symbol->next_use;
            }

            if(x) {
                if(!ll_find(code_block->vars, c->x->value.symbol, pointer_match)) {
                    code_block->vars = ll_insertfront(code_block->vars, c->x->value.symbol);
                }

                c->x_live = c->x->value.symbol->live;
                c->x_next = c->x->value.symbol->next_use; 
            }

            if(y) {
                if(!ll_find(code_block->vars, c->y->value.symbol, pointer_match)) {
                    code_block->vars = ll_insertfront(code_block->vars, c->y->value.symbol);
                }
        
                c->y_live = c->y->value.symbol->live;
                c->y_next = c->y->value.symbol->next_use; 
            }


            /*
                2. In the symbol table, set x to "not live" and "no next use."
            */
            if(result) {
                c->result->value.symbol->live = false;
                c->result->value.symbol->next_use = 0;
            }

            /*
                3. In the symbol table, set y and z to "live" and the next uses of y and z to i.
            */

            if(x) {
                c->x->value.symbol->live = false;
                c->x->value.symbol->next_use = 0;
            }

            if(y) {
                c->y->value.symbol->live = false;
                c->y->value.symbol->next_use = 0;
            }
        }

        statements = ll_next(statements);
    }

    // LINKED_LIST* vars = code_block->vars;

    // while(vars) {
    //     SYMBOL* s = ll_value(vars);
    //     if(s->live) {
    //         code_block->live_on_exit = ll_insertfront(code_block->live_on_exit, s);
    //         print_symbol(s, code_block->label, stdout);
    //     }
    //     vars = ll_next(vars);
    // }
}

LINKED_LIST* make_blocks(INTERMEDIATE_CODE* code_table) {

    LINKED_LIST* blocks = NULL;
    BLOCK* current_block = NULL;

    mark_leaders(code_table);
    
    for(int i = 0; i < code_table->next_instruction; i++) {
        TAC* code = code_table->entries[i];
        if(code) {
            if(code->leader) {
                if(current_block) {
                    set_next_use_information(current_block);
                    ll_reverse(&current_block->code);
                }
                current_block = new_code_block(code->label);
                blocks = ll_insertfront(blocks, current_block);
            }

            // All symbols should be live when they exit the block
            if(code->x && code->x->meta == AT_SYMBOL) {
                code->x->value.symbol->live = true;
            }

            if(code->y && code->y->meta == AT_SYMBOL) {
                code->y->value.symbol->live = true;
            }

            if(code->result && code->result->meta == AT_SYMBOL) {
                code->result->value.symbol->live = true;
            }

            current_block->code = ll_insertfront(current_block->code, code);
        }
    }
    ll_reverse(&blocks);
    LINKED_LIST *head = blocks;
    while(head){
		BLOCK *cb = head->value;
		cb->optimizer = create_optimize_graph(cb);
		head = ll_next(head); 
	}
    return blocks;
}

GRAPH *create_optimize_graph(BLOCK *code_block){
	GRAPH *gr = newGraph();
	LINKED_LIST *code_list = code_block->code;
	while(code_list){
		TAC *code = ll_value(code_list);
		process_tac(gr,code);
		code_list = ll_next(code_list);
	}
	return gr;
}

void common_expression_optimize(LINKED_LIST *code_list){
	while(code_list){
		BLOCK *code_block = code_list->value;
     	optimize_common_exp(code_block->optimizer);
     	code_list = ll_next(code_list);
     }
}

void dead_code_optimize(LINKED_LIST *code_list){
	while(code_list){
		BLOCK *code_block = code_list->value;
     	optimize_dead_code(code_block->optimizer);
     	code_list = ll_next(code_list);
     }
}

void mark_leaders(INTERMEDIATE_CODE* code_table) {

    for(int i = 0; i < code_table->next_instruction; i++) {
        TAC* code = code_table->entries[i];
        if(code_is_jump(code)) {
            code->leader = true;

            if(code->op != I_RETURN) {
                ADDRESS* label = code->result;

                if(code->op == I_CALL) {
                    label = code->x->value.symbol->label;
                }

                if(label) {
                    code_table->entries[label->value.label]->leader = true;
                }
            }

            if(i + 1 < code_table->next_instruction) {
                code_table->entries[i + 1]->leader = true;
            }
        }
    }
}
