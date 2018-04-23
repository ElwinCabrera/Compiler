#include <stdlib.h>
#include <string.h>
#include "assembly.h"
#include "code_blocks.h"

static LINKED_LIST* assembly_code;


void create_assembly_block(BLOCK* block) {

    if(!block) {
        return;
    }

    LINKED_LIST* code_list = block->code;

    while(code_list) {
        TAC* code = ll_value(code_list);
        int rd = -1;
        int rs1 = -1;
        int rs2 = -1;

        get_reg(code, &rd, &rs1, &rs2);
        create_assembly(block->label, code, rd, rs1, rs2);
    }

}

void create_assembly(int label, TAC* code, REG rd, REG rs1, REG rs2) {

}

ASSEMBLY* new_asm() {
    ASSEMBLY* asm_code = malloc(sizeof(ASSEMBLY));
    memset(asm_code, 0, sizeof(ASSEMBLY));
    return asm_code;
}

void add_atype(ASM_OP op, REG rd, REG rs1, REG rs2, bool s, bool c, CONDITION cond) {
    ASSEMBLY* a = new_asm();
    a->type = IT_A;
    a->op = op;
    a->rd = rd;
    a->rs1 = rs1;
    a->rs2 = rs2;
    a->cond = cond;
    a->s = s;
    assembly_code = ll_insertfront(assembly_code, a);
}

/*
    Probably want to restrict this int to a short
*/
void add_itype(ASM_OP op, REG rd, REG rs1, int immediate) {
    ASSEMBLY* a = new_asm();
    a->type = IT_I;
    a->op = op;
    a->rd = rd;
    a->rs1 = rs1;
    a->immediate = immediate;
    assembly_code = ll_insertfront(assembly_code, a);
}

void add_jtype(int immediate) {
    ASSEMBLY* a = new_asm();
    a->type = IT_J;
    a->op = ASM_JMP;
    a->immediate = immediate;
    assembly_code = ll_insertfront(assembly_code, a);
}

void add_btype(ASM_OP op, int immediate, bool c, CONDITION cond) {
    ASSEMBLY* a = new_asm();
    a->type = IT_B;
    a->op = op;
    a->immediate = immediate;
    a->c = c;
    a->cond = cond;
    assembly_code = ll_insertfront(assembly_code, a);
}
