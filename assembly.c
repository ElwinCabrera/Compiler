#include <stdlib.h>
#include <string.h>
#include "assembly.h"
#include "code_blocks.h"

static LINKED_LIST* assembly_code;

void process_code_blocks(LINKED_LIST* blocks) {

    while(blocks) {
        create_assembly_block(ll_value(blocks));
        blocks = ll_next(blocks);
    }
}

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
        code_list = ll_next(code_list);
    }

}

void create_assembly(int label, TAC* code, REG rd, REG rs1, REG rs2) {
    printf("Hello\n");
}

void print_asm_code(FILE* f) {
    if(!f) {
        f = stdout;
    }

    int label = -1;

    LINKED_LIST* asm_list = assembly_code;

    while(asm_list) {
        ASSEMBLY* a = ll_value(asm_list);
        if(a->label != label) {
            label = a->label;
            fprintf(f, "LABEL%d:", label);
        }
        
        switch(a->type) {
            case IT_A:
                fprintf(f, "\t\t %s%s%s%s R%d R%d", 
                    get_asm_mnemonic(a->op),
                    get_condition_str(a->cond),
                    a->s ? "S" : "",
                    a->c ? "C" : "",
                    a->rd,
                    a->rs1);
                if(a->rs2 >= 0) {
                    fprintf(f, " R%d", a->rs2);
                }
                fprintf(f, "\n");
                break;
            case IT_I:
                fprintf(f, "\t\t %s R%d R%d #%d\n", 
                    get_asm_mnemonic(a->op),
                    a->rd,
                    a->rs1,
                    a->immediate);
                break;
            case IT_J:
                fprintf(f, "\t\t J %d\n", a->immediate);
                break;
            case IT_B:
                fprintf(f, "\t\t %s%s%s %d\n", 
                    get_asm_mnemonic(a->op),
                    get_condition_str(a->cond),
                    a->c ? "C" : "",
                    a->immediate);
                break;
        }

        asm_list = ll_next(asm_list);
    }


}

const char* get_condition_str(CONDITION cond) {
    switch(cond) {
        case AL: return "";
        case EQ: return "EQ";
        case NE: return "NE";
        case CA: return "CA";
        case CC: return "CC";
        case NG: return "NG";
        case PZ: return "PZ";
        case VS: return "VS";
        case VC: return "VC";
        case HI: return "HI";
        case LS: return "LS";
        case GE: return "GE";
        case LT: return "LT";
        case GT: return "GT";
        case LE: return "LE";
        default: return NULL;
    }
}

const char* get_asm_mnemonic(ASM_OP op) {
    switch(op) {
        case ASM_NOP: return "NOP";    
        case ASM_ADD: return "ADD";    
        case ASM_AND: return "AND";    
        case ASM_ASR: return "ASR";    
        case ASM_LSL: return "LSL";    
        case ASM_LSR: return "LSR";    
        case ASM_NAND: return "NAND";  
        case ASM_NOR: return "NOR";   
        case ASM_NOT: return "NOT";    
        case ASM_OR: return "OR";     
        case ASM_SUB: return "SUB";    
        case ASM_XNOR: return "XNOR";   
        case ASM_XOR: return "XOR";    
        case ASM_ADDI: return "ADDI";   
        case ASM_ANDI: return "ANDI";   
        case ASM_ASRI: return "ASRI";
        case ASM_LDR: return "LDR"; 
        case ASM_LDRB: return "LDRB"; 
        case ASM_LDRBU: return "LDRBU";
        case ASM_LDRH: return "LDRH"; 
        case ASM_LDRHU: return "LDRHU"; 
        case ASM_LSLI: return "LSLI";
        case ASM_LSRI: return "LSRI";  
        case ASM_LUI: return "LUI";
        case ASM_NANDI: return "NANDI";  
        case ASM_NORI: return "NORI";
        case ASM_ORI: return "ORI";    
        case ASM_STR: return "STR";
        case ASM_STRB: return "STRB"; 
        case ASM_STRH: return "STRH"; 
        case ASM_SUBI: return "SUBI";   
        case ASM_XNORI: return "XNORI";  
        case ASM_XORI: return "XORI";   
        case ASM_JMP: return "JMP";
        case ASM_BRANCH: return "B";     
        case ASM_BRANCHL: return "BL";
        case ASM_BRANCHR: return "BR";
        default: return NULL;
    }
}

ASSEMBLY* new_asm(int label) {
    ASSEMBLY* asm_code = malloc(sizeof(ASSEMBLY));
    memset(asm_code, 0, sizeof(ASSEMBLY));
    asm_code->label = label;
    return asm_code;
}

void add_atype(int label, ASM_OP op, REG rd, REG rs1, REG rs2, bool s, bool c, CONDITION cond) {
    ASSEMBLY* a = new_asm(label);
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
void add_itype(int label, ASM_OP op, REG rd, REG rs1, int immediate) {
    ASSEMBLY* a = new_asm(label);
    a->type = IT_I;
    a->op = op;
    a->rd = rd;
    a->rs1 = rs1;
    a->immediate = immediate;
    assembly_code = ll_insertfront(assembly_code, a);
}

void add_jtype(int label, int immediate) {
    ASSEMBLY* a = new_asm(label);
    a->type = IT_J;
    a->op = ASM_JMP;
    a->immediate = immediate;
    assembly_code = ll_insertfront(assembly_code, a);
}

void add_btype(int label, ASM_OP op, int immediate, bool c, CONDITION cond) {
    ASSEMBLY* a = new_asm(label);
    a->type = IT_B;
    a->op = op;
    a->immediate = immediate;
    a->c = c;
    a->cond = cond;
    assembly_code = ll_insertfront(assembly_code, a);
}
