#include <stdlib.h>
#include "code_blocks.h"
#include "linked_list.h"
#include "address.h"
#include "symbol_table.h"

BLOCK* new_code_block(int i) {
    BLOCK* b = malloc(sizeof(BLOCK));
    b->code = NULL;
    b->label = i;
    return b;
}

void print_blocks(LINKED_LIST* l, FILE* f) {
    ll_reverse(&l);
    while(l) {
        BLOCK* b = ll_value(l);
        fprintf(f ? f : stdout, "Block %d\n", b->label);
        LINKED_LIST* code_list = b->code;
        ll_reverse(&code_list);
        while(code_list) {
            TAC* code = ll_value(code_list);
            print_tac(code, f);
            code_list = ll_next(code_list);
        }
        fprintf(f ? f : stdout, "\n");
        l = ll_next(l);
    }
}

LINKED_LIST* make_blocks(INTERMEDIATE_CODE* code_table) {

    LINKED_LIST* blocks = NULL;
    BLOCK* current_block;

    mark_leaders(code_table);
    
    for(int i = 0; i < code_table->next_instruction; i++) {
        TAC* code = code_table->entries[i];
        if(code) {
            if(code->leader) {
                current_block = new_code_block(code->label);
                blocks = ll_insertfront(blocks, current_block);
            }
            current_block->code = ll_insertfront(current_block->code, code);
        }
    }

    return blocks;
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

                code_table->entries[label->value.label]->leader = true;
            }

            if(i + 1 < code_table->next_instruction) {
                code_table->entries[i + 1]->leader = true;
            }
        }
    }
}