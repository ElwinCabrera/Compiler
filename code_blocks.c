#include "code_blocks.h"
#include "linked_list.h"
#include "address.h"
#include "symbol_table.h"

LINKED_LIST* make_blocks(INTERMEDIATE_CODE* code_table) {

    mark_leaders(code_table);
    
    return NULL;
}

void mark_leaders(INTERMEDIATE_CODE* code_table) {

    for(int i = i; i < code_table->next_instruction; i++) {
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