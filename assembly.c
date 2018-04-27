#include <stdlib.h>
#include <string.h>
#include "assembly.h"
#include "code_blocks.h"
#include "intermediate_code.h"
#include "address.h"
#include "data_block.h"
#include "registers.h"
#include "location.h"
#include "types.h"

static LINKED_LIST* assembly_code = NULL;

void process_code_blocks(LINKED_LIST* blocks) {

    while(blocks) {
        create_assembly_block(ll_value(blocks));
        blocks = ll_next(blocks);
    }

    ll_reverse(&assembly_code);
}

void create_assembly_block(BLOCK* block) {

    if(!block) {
        return;
    }

    LINKED_LIST* code_list = block->code;

    while(code_list) {
        TAC* code = ll_value(code_list);
        create_assembly(block->label, code);
        code_list = ll_next(code_list);
    }

    clear_temporary_registers();
}

void asm_stack_variables(int block, SCOPE* s) {
    /*
        Whenever you enter a scope that has sybols, allocate
        space on the stack for those
    */
}

void asm_function_setup(int block, SYMBOL* fn) {

    /*
        Anything that needs to be done before the 
        stack vars are declared
    */

    if(!fn) {
        return;
    }

}

void asm_parameter(int block, ADDRESS* a) {
    /*
        Can pick the argument registers if they're empty,
        otherwise put onto stack frame
    */

    //REG rs = get_parameter_register(a);
}

void asm_function_call(int block, TAC* code) {

    /*
        Params have been pushed, this just needs to adjust
        the stack (write link, etc) and jump

        Anything that can be done here in terms of setup 
        instead of callee (asm_function_setup) saves instruction 
        space
    */

    SYMBOL* fn = code->x->value.symbol;

    if(!fn) {
        return;
    }
    
    // Space for return value
    add_itype(block, ASM_ADDI, STACK, STACK, const_location(4));

    // Write the control link

    add_itype(block, ASM_ADDI, STACK, STACK, const_location(4));
    add_itype(block, ASM_STR, STACK, PC, const_location(0));


    add_btype(block, ASM_BRANCHL, fn->label->value.label, false, 0);

    /*
        Next instruction is executed on return from the function:
        Return stack to control link address and then place the return 
        value into the temporary.
    */
    
}

void asm_function_return(int block, SYMBOL* fn) {

    if(!fn) {
        return;
    }

    LINKED_LIST* ret_location = fn->address_descriptor;

    if(!ret_location) {
        return;
    }

    add_btype(block, ASM_BRANCHR, 0, false, 0);
}

void asm_heap_reserve(int block, ADDRESS* a, int size) {
    
    /*
        Put the heap address into RD, and increment HEAP
        by reverse amount.
    */
    
    REG r = get_dest_register(a);
    add_atype(block, ASM_ADD, r, HEAP, ZERO, false, false, false);
    add_itype(block, ASM_ADDI, HEAP, HEAP, const_location(size));
}


void asm_assignment(int block, TAC* code) {
    /*
        RD = RS1
    */

    REG rs1 = get_source_register(code->x);
    REG rd = get_dest_register(code->result);

    if(rs1 == NO_REGISTER) {
        add_itype(block, ASM_ADDI, rd, ZERO, const_location(code->x->value.integer));
    } else {
        add_atype(block, ASM_ADD, rd, rs1, ZERO, false, false, false);
    }
}


void asm_add(int block, TAC* code) {
    /*
        RD = RS1 + RS2
    */
    REG rs1 = get_source_register(code->x);
    REG rs2 = get_source_register(code->y);
    REG rd = get_dest_register(code->result);
    
    if(check_typename(code->result->type, "integer")) {
        if(rs1 == NO_REGISTER) {
            add_itype(block, ASM_ADDI, rd, rs2, const_location(code->x->value.integer));
        } else if(rs2 == NO_REGISTER) {
            add_itype(block, ASM_ADDI, rd, rs1, const_location(code->y->value.integer));
        } else {
            add_atype(block, ASM_ADD, rd, rs1, rs2, false, false, false);
        }
    } else {
        /*
            Floating point arithmetic
        */
    }

}

void asm_sub(int block, TAC* code) {
    /*
        RD = RS1 - RS2
    */

    REG rs1 = get_source_register(code->x);
    REG rs2 = get_source_register(code->y);
    REG rd = get_dest_register(code->result);

    if(check_typename(code->result->type, "integer")) {
        if(rs1 == NO_REGISTER && rs2 == NO_REGISTER) {
            // Unary minus
            add_itype(block, ASM_ADDI, POSNEG, ZERO, const_location(code->x->value.integer));
            add_atype(block, ASM_ADD, rd, NEGPOS, ZERO, false, false, false);
        } else if(rs1 == NO_REGISTER) {
            add_itype(block, ASM_ADDI, POSNEG, rs2, const_location(code->x->value.integer));
            add_atype(block, ASM_ADD, rd, NEGPOS, rs1, false, false, false);
        } else if(rs2 == NO_REGISTER) {
            add_itype(block, ASM_SUBI, rd, rs1, const_location(code->y->value.integer));
        } else {
            add_atype(block, ASM_SUB, rd, rs1, rs2, false, false, false);
        }
    } else {
        /*
            Floating point arithmetic
        */
    }
}

void asm_multiply(int block, TAC* code) {
    /*
        JMP to multiply routine
    */
}

void asm_divide(int block, TAC* code) {
    /*
        JMP to divide routine
    */
}

void asm_modulus(int block, TAC* code) {
    /*
        JMP to modulus routine
    */
}

void asm_less_than(int block, TAC* code) {

    REG rs1 = get_source_register(code->x);
    REG rs2 = get_source_register(code->y);
    REG rd = get_dest_register(code->result);

    if(rs1 == NO_REGISTER) {

    } else if(rs2 == NO_REGISTER) {

    } else {
        add_atype(block, I_SUB, rd, rs1, rs2, true, false, false);
        add_atype(block, I_ADD, rd, ZERO, ZERO, false, false, PZ);
    }
}

void asm_equal(int block, TAC* code) {
    
    REG rs1 = get_source_register(code->x);
    REG rs2 = get_source_register(code->y);
    REG rd = get_dest_register(code->result);

    if(rs1 == NO_REGISTER) {

    } else if(rs2 == NO_REGISTER) {

    }

}

void asm_not(int block, TAC* code) {

    REG rs = get_source_register(code->x);
    REG rd = get_dest_register(code->result);
    if(rs == NO_REGISTER) {
        if(code->x->value.boolean) {
            add_atype(block, ASM_NOT, rd, ZERO, NO_REGISTER, false, false, false);
        } else {
            add_atype(block, ASM_ADD, rd, ZERO, ZERO, false, false, false);
        }
    } else {
        add_atype(block, ASM_NOT, rd, rs, NO_REGISTER, false, false, false);
    }

}

void asm_conditional_branch(int block, TAC* code) {
    REG rs = get_source_register(code->x);
    if(rs == NO_REGISTER) {
            /*
            Boolean constant in test case:
            If it's true, insert unconditional jump,
            otherwise insert nothing. Note we end up w/ dead code
        */
        if(code->x->value.boolean) {
            add_btype(block, ASM_BRANCH, code->result->value.label, false, false);
        }
    } else {
        add_atype(block, ASM_AND, rs, rs, ZERO, true, false, false);
        add_btype(block, ASM_BRANCH, code->result->value.label, false, NE);
    }
}

void asm_conditional_branch_false(int block, TAC* code) {
    REG rs = get_source_register(code->x);
    if(rs == NO_REGISTER) {
        /*
            Boolean constant in test case:
            If it's false, insert unconditional jump,
            otherwise insert nothing. Note we end up w/ dead code
        */
        if(!code->x->value.boolean) {
            add_btype(block, ASM_BRANCH, code->result->value.label, false, false);
        }
    } else {
        add_atype(block, ASM_AND, rs, rs, ZERO, true, false, false);
        add_btype(block, ASM_BRANCH, code->result->value.label, false, EQ);
    }
}

void asm_conditional_branch_not_equal(int block, TAC* code) {
    REG rs1 = get_source_register(code->x);
    REG rd = get_dest_register(code->x);
    add_itype(block, ASM_ADDI, rd, ZERO, const_location(code->y->value.integer));
    add_atype(block, ASM_SUB, rd, rs1, rd, true, false, false);
    add_btype(block, ASM_BRANCH, code->result->value.label, false, EQ);
}

void asm_array_access(int block, TAC* code) {
    REG rs1 = get_source_register(code->x);
    REG rd = get_dest_register(code->result);
    int width = get_type_width(code->x->type->element_type);
    int dimension_spots = code->x->type->dimensions * 4;
    add_itype(block, ASM_LDR, rd, rs1, memory_location(12 + dimension_spots + (width * code->y->value.integer)));
}

void asm_array_assign(int block, TAC* code) {
    REG rs1 = get_source_register(code->y);
    REG rd = get_dest_register(code->result);
    int width = get_type_width(code->result->type->element_type);
    int dimension_spots = code->result->type->dimensions * 4;
    add_itype(block, ASM_ADDI, rd, rd, memory_location(4 + dimension_spots + (width * code->x->value.integer)));
    if(rs1 == NO_REGISTER) {
        add_itype(block, ASM_STR, rd, ZERO, const_location(code->y->value.integer));
    } else {
        add_itype(block, ASM_STR, rd, rs1, const_location(0));
    }
}

void asm_record_access(int block, TAC* code) {
    REG rs1 = get_source_register(code->x);
    REG rd = get_dest_register(code->result);

    LINKED_LIST* symbols = code->x->type->members ? code->x->type->members->symbols : NULL;
    int offset = 0;
    while(symbols) {
        SYMBOL* s = ll_value(symbols);
        if(s == code->y->value.symbol) {
            break;
        } else {
            offset += get_type_width(s->type);
        }
        symbols = ll_next(symbols);
    }

    add_itype(block, ASM_LDR, rd, rs1, memory_location(offset));
}

void asm_record_assign(int block, TAC* code) {
    REG rs1 = get_source_register(code->x);
    REG rd = get_dest_register(code->result);

    LINKED_LIST* symbols = code->result->type->members ? code->result->type->members->symbols : NULL;
    int offset = 0;
    while(symbols) {
        SYMBOL* s = ll_value(symbols);
        if(s == code->y->value.symbol) {
            break;
        } else {
            offset += get_type_width(s->type);
        }
        symbols = ll_next(symbols);
    }

    add_itype(block, ASM_ADDI, rd, rd, memory_location(offset));
    add_itype(block, ASM_STR, rd, rs1, memory_location(0));
}

void create_assembly(int label, TAC* code) {
    
    switch(code->op) {
        case I_NOP:
            add_nop(label);
            break;
        case I_STACK_VARS:
            asm_stack_variables(label, code->result->value.scope);
            break;
        case I_CALL:
            asm_function_call(label, code);
            break;
        case I_FN_START:
            asm_function_setup(label, code->result->value.symbol);
            break;
        case I_RETURN:
            asm_function_return(label, code->result->value.symbol);
            break;
        case I_GOTO:
            add_btype(label, ASM_JMP, code->result->value.label, false, 0);
            break;
        case I_ASSIGN:
            asm_assignment(label, code);
            break;
        case I_PARAM:
            asm_parameter(label, code->x);
            break;
        case I_RESERVE:
            asm_heap_reserve(label, code->result, code->x->value.integer);
            break;
        case I_RELEASE:
            // Nothing
            break;
        case I_TEST:
            asm_conditional_branch(label, code);
            break;
        case I_TEST_FALSE:
            asm_conditional_branch_false(label, code);
            break;
        case I_TEST_NOTEQUAL:
            asm_conditional_branch_not_equal(label, code);
            break;
        case I_RECORD_ACCESS:
            asm_record_access(label, code);
            break;
        case I_RECORD_ASSIGN:
            asm_record_assign(label, code);
            break;
        case I_ARRAY_ACCESS:
            asm_array_access(label, code);
            break;
        case I_ARRAY_ASSIGN:
            asm_array_assign(label, code);
            break;
        case I_ADD:
            asm_add(label, code);
            break;
        case I_SUB:
            asm_sub(label, code);
            break;
        case I_MULTIPLY:
            asm_multiply(label, code);
            break;
        case I_DIVIDE:
            asm_divide(label, code);
            break;
        case I_MODULUS:
            asm_modulus(label, code);
            break;
        case I_LESS_THAN:
            asm_less_than(label, code);
            break;
        case I_EQUAL:
            asm_equal(label, code);
            break;
        case I_NOT:
            asm_not(label, code);
            break;
        case I_INT2REAL:
            break;
        case I_REAL2INT:
            break;
        default: 
            printf("Unsupported ASM translation: %d\n", code->op);
            break;
    }

}

void print_asm_code(FILE* f) {
    if(!f) {
        f = stdout;
    }

    int label = 0;

    print_data_block(f);

    fprintf(f, "MAIN");

    LINKED_LIST* asm_list = assembly_code;

    while(asm_list) {
        ASSEMBLY* a = ll_value(asm_list);
        
        if(a->label != label) {
            label = a->label;
            fprintf(f, "\nLABEL%02d ", label);
        } else {
            fprintf(f, "\t");
        }
        
        switch(a->type) {
            case IT_NOP:
                fprintf(f, "\t\t NOP 0 0 0\n");
                break;
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
            case IT_I: {
                char* immediate = create_location_str(a->immediate);
                fprintf(f, "\t\t %s R%d R%d %s\n", 
                    get_asm_mnemonic(a->op),
                    a->rd,
                    a->rs1,
                    immediate);
                free(immediate);
                break;
            }
            case IT_J:
                fprintf(f, "\t\t J LABEL%d\n", a->immediate->value.constant);
                break;
            case IT_B:
                fprintf(f, "\t\t %s%s%s", 
                    get_asm_mnemonic(a->op),
                    get_condition_str(a->cond),
                    a->c ? "C" : "");
                if(a->immediate) {
                    fprintf(f, " LABEL%02d",  a->immediate->value.constant);
                }
                fprintf(f, "\n");
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
        case ASM_JMP: return "J";
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

void add_nop(int label) {
    ASSEMBLY* a = new_asm(label);
    a->op = ASM_NOP;
    a->type = IT_NOP;
    assembly_code = ll_insertfront(assembly_code, a);
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
void add_itype(int label, ASM_OP op, REG rd, REG rs1, LOCATION* immediate) {
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
    a->immediate = const_location(immediate);
    assembly_code = ll_insertfront(assembly_code, a);
}

void add_btype(int label, ASM_OP op, int immediate, bool c, CONDITION cond) {
    ASSEMBLY* a = new_asm(label);
    a->type = IT_B;
    a->op = op;
    if(immediate) {
        a->immediate = const_location(immediate);
    }
    a->c = c;
    a->cond = cond;
    assembly_code = ll_insertfront(assembly_code, a);
}
