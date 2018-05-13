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
#include "stack.h"

static bool USED_DIVIDE = false;
static bool USED_MULTIPLY = false;

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

int compute_stack_space(SCOPE* s, int base_offset) {

    if(!s) {
        return base_offset;
    }

    reorder_symbols(s);

    LINKED_LIST* syms = s->symbols;

    int space_needed = base_offset;

    while(syms) {
        SYMBOL* s = ll_value(syms);
        s->stack_offset = space_needed;
        // printf("Offset for %s: %d\n", s->name, space_needed);
        space_needed += get_type_width(s->type);
        syms = ll_next(syms);
    }


    int pad_4 = (space_needed % 4);

    if(pad_4 > 0) {
        space_needed += (4 - pad_4);
    }

    LINKED_LIST* children = s->children;
    
    int children_max = space_needed;
    
    while(children) {
        SCOPE* child_s = ll_value(children);
        int width = compute_stack_space(child_s, space_needed);
        
        if(width > children_max) {
            children_max = width;
        }

        children = ll_next(children);
    }

    return children_max;
}

void asm_stack_variables(int block, SCOPE* s) {
    /*
        Whenever you enter a scope that has sybols, allocate
        space on the stack for those
    */
    int space = compute_stack_space(s, 0);
    add_itype(block, ASM_ADDI, TOP_SP, SP, const_location(0));
    add_itype(block, ASM_ADDI, SP, TOP_SP, const_location(space));
    // printf("Needed stack space for scope %d: %d\n", s->id, space);
}

void asm_function_setup(int block, SYMBOL* fn) {

    /*
        Anything that needs to be done immediately
        before control transfers to callee
    */

    if(!fn) {
        return;
    }

}

STACK* params = NULL;

void asm_parameter(int block, ADDRESS* a) {
    /*
        Can pick the argument registers if they're empty,
        otherwise put onto stack frame
    */

    //REG rs = get_parameter_register(a);
    params = stack_push(params, a);
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


    // Write the control link


    add_btype(block, ASM_BRANCHL, label_location(fn->label->value.label), false, 0);

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
    add_itype(block, ASM_SUBI, HEAP, HEAP, const_location(size));
}


void asm_assignment(int block, TAC* code) {
    /*
        RD = RS1
        or
        RD = constant
    */

    REG rs1 = get_source_register(code->x);

    if(rs1 == CONST_VALUE) {
        REG rd = get_dest_register(code->result);
        add_itype(block, ASM_ADDI, rd, ZERO, const_location(code->x->value.integer));
    } else {
        LOCATION* l = register_location(rs1);
        code->result->value.symbol->address_descriptor = ll_insertfront(code->result->value.symbol->address_descriptor, l);
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
        if(rs1 == CONST_VALUE) {
            add_itype(block, ASM_ADDI, rd, rs2, const_location(code->x->value.integer));
        } else if(rs2 == CONST_VALUE) {
            add_itype(block, ASM_ADDI, rd, rs1, const_location(code->y->value.integer));
        }else if (rs1 == CONST_VALUE && rs2 == CONST_VALUE ){

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

         if(rs2 == NULL_ADDRESS) {
			// Unary minus
			add_itype(block, ASM_ADDI, POSNEG, ZERO, const_location(code->x->value.integer));
            add_atype(block, ASM_ADD, rd, NEGPOS, ZERO, false, false, false);
         } else {
         	if(rs1 == CONST_VALUE){
         		add_itype(block, ASM_SUBI, rd, rs2, const_location(code->x->value.integer));
         	} else if (rs2 == CONST_VALUE) {
         		add_itype(block, ASM_SUBI, rd, rs1, const_location(code->y->value.integer));
         	}else {
             add_atype(block, ASM_SUB, rd, rs1, rs2, false, false, false);
            }
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
    REG rd = get_dest_register(code->result);
    REG rs1 = get_source_register(code->x);
    REG rs2 = get_source_register(code->y);
    
    if(check_typename(code->result->type, "integer")) {
        if(rs1 == CONST_VALUE) {
            add_itype(block, ASM_ADDI, ARG0, ZERO, const_location(code->x->value.integer));
        } else {
            add_atype(block, ASM_ADD, ARG0, rs1, ZERO, false, false, false);
        }
    
        if(rs2 == CONST_VALUE) {
            add_itype(block, ASM_ADDI, ARG1, ZERO, const_location(code->y->value.integer));
        } else {
            add_atype(block, ASM_ADD, ARG1, rs2, ZERO, false, false, false);
        }

        
        add_btype(block, ASM_BRANCHL, strlabel_location("multiply"), false, false); 
        add_atype(block, ASM_ADD, rd, T2, ZERO, false, false, false);
    } else {
        add_atype(block, ASM_AND, ZERO, ZERO, ZERO, false, false, false);
    }
    
}

void asm_divide(int block, TAC* code) {
    /*
        JMP to divide routine
    */
    
    REG rd = get_dest_register(code->result);
    REG rs1 = get_source_register(code->x);
    REG rs2 = get_source_register(code->y);
    
    if(check_typename(code->result->type, "integer")) {
        if(rs1 == CONST_VALUE) {
            add_itype(block, ASM_ADDI, ARG0, ZERO, const_location(code->x->value.integer));
        } else {
            add_atype(block, ASM_ADD, ARG0, rs1, ZERO, false, false, false);
        }
    
        if(rs2 == CONST_VALUE) {
            add_itype(block, ASM_ADDI, ARG1, ZERO, const_location(code->y->value.integer));
        } else {
            add_atype(block, ASM_ADD, ARG1, rs2, ZERO, false, false, false);
        }

        add_btype(block, ASM_BRANCHL, strlabel_location("div_and_mod"), false, false); 
        add_atype(block, ASM_ADD, rd, S0, ZERO, false, false, false);
    } else {
        add_atype(block, ASM_AND, rd, ZERO, ZERO, false, false, false);
    }
     
}

void asm_modulus(int block, TAC* code) {
    /*
        JMP to modulus routine
    */
    
    REG rd = get_dest_register(code->result);
    REG rs1 = get_source_register(code->x);
    REG rs2 = get_source_register(code->y);
    
    if(check_typename(code->result->type, "integer")) {
        if(rs1 == CONST_VALUE) {
            add_itype(block, ASM_ADDI, ARG0, ZERO, const_location(code->x->value.integer));
        } else {
            add_atype(block, ASM_ADD, ARG0, rs1, ZERO, false, false, false);
        }
    
        if(rs2 == CONST_VALUE) {
            add_itype(block, ASM_ADDI, ARG1, ZERO, const_location(code->y->value.integer));
        } else {
            add_atype(block, ASM_ADD, ARG1, rs2, ZERO, false, false, false);
        }

        add_btype(block, ASM_BRANCHL, strlabel_location("div_and_mod"), false, false); 
        add_atype(block, ASM_ADD, rd, S1, ZERO, false, false, false);

    } else {
        add_atype(block, ASM_AND, rd, ZERO, ZERO, false, false, false);
    }


}

void asm_less_than(int block, TAC* code) {

    REG rs1 = get_source_register(code->x);
    REG rs2 = get_source_register(code->y);
    REG rd = get_dest_register(code->result);

}

void asm_equal(int block, TAC* code) {
    
     REG rs1 = get_source_register(code->x);
     REG rs2 = get_source_register(code->y);
     REG rd = get_dest_register(code->result);
     
     add_itype(block, ASM_ADDI, rd, ZERO, const_location(0));
     
     if (rs1 == CONST_VALUE) {
     	add_itype(block, ASM_ADDI, rd, ZERO, const_location(code->x->value.integer));
     	add_atype(block, ASM_SUB, rd, rd, rs2, true, false, false);
     	
     	 
     } else if (rs2 == CONST_VALUE){
     	add_itype(block, ASM_ADDI, rd, ZERO, const_location(code->y->value.integer));
     	add_atype(block, ASM_SUB, rd, rs1, rd, true, false, false);
     	 
     } else if (rs1 == CONST_VALUE && rs2 == CONST_VALUE){
     	add_itype(block, ASM_ADDI, LINK1, ZERO, const_location(code->x->value.integer));
     	add_itype(block, ASM_ADDI, LINK2, ZERO, const_location(code->y->value.integer));
     	add_atype(block, ASM_SUB, rd, LINK1, LINK2, true, false, false);
     	
     } else {
     	add_atype(block, ASM_SUB, rd, rs1, rs2, true, false, false);
     }
     
     add_itype(block, ASM_ADDI, rd, ZERO, const_location(4));
     add_atype(block, ASM_AND, rd, rd, CPSR, false, false, false);
     
     add_itype(block, ASM_ADDI, LINK1, ZERO, const_location(2));
     add_atype(block, ASM_LSR, rd, rd, LINK1, false, false, false);
  
}

void asm_not(int block, TAC* code) {

    REG rs = get_source_register(code->x);
    REG rd = get_dest_register(code->result);
    if(rs == CONST_VALUE) {
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
    if(rs == CONST_VALUE) {
            /*
            Boolean constant in test case:
            If it's true, insert unconditional jump,
            otherwise insert nothing. Note we end up w/ dead code
        */
        if(code->x->value.boolean) {
            add_btype(block, ASM_BRANCH, label_location(code->result->value.label), false, false);
        } else {
            add_atype(block, ASM_ADD, ZERO, ZERO, ZERO, false, false, false);
        }
    } else {
        add_atype(block, ASM_AND, rs, rs, ZERO, true, false, false);
        add_btype(block, ASM_BRANCH, label_location(code->result->value.label), false, NE);
    }
}

void asm_conditional_branch_false(int block, TAC* code) {
    REG rs = get_source_register(code->x);
    if(rs == CONST_VALUE) {
        /*
            Boolean constant in test case:
            If it's false, insert unconditional jump,
            otherwise insert nothing. Note we end up w/ dead code
        */
        if(!code->x->value.boolean) {
            add_btype(block, ASM_BRANCH, label_location(code->result->value.label), false, false);
        } else {
            add_atype(block, ASM_ADD, ZERO, ZERO, ZERO, false, false, false);
        }
    } else {
        add_atype(block, ASM_AND, rs, rs, ZERO, true, false, false);
        add_btype(block, ASM_BRANCH, label_location(code->result->value.label), false, EQ);
    }
}

void asm_conditional_branch_not_equal(int block, TAC* code) {
    REG rs1 = get_source_register(code->x);
    REG rd = get_dest_register(code->x);
    add_itype(block, ASM_ADDI, rd, ZERO, label_location(code->y->value.integer));
    add_atype(block, ASM_SUB, rd, rs1, rd, true, false, false);
    add_btype(block, ASM_BRANCH, label_location(code->result->value.label), false, EQ);
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
    if(rs1 == CONST_VALUE) {
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
            add_jtype(label, label_location(code->result->value.label));
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
            add_atype(label, ASM_ADD, ZERO, ZERO, ZERO, false, false, false);
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
            USED_MULTIPLY = true;
            asm_multiply(label, code);
            break;
        case I_DIVIDE:
            USED_DIVIDE = true;
            asm_divide(label, code);
            break;
        case I_MODULUS:
            USED_DIVIDE = true;
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
            add_atype(label, ASM_ADD, ZERO, ZERO, ZERO, false, false, false);
            break;
        case I_REAL2INT:
            add_atype(label, ASM_ADD, ZERO, ZERO, ZERO, false, false, false);
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

    fprintf(f, "MAIN\t");

    LINKED_LIST* asm_list = assembly_code;

    while(asm_list) {
        ASSEMBLY* a = ll_value(asm_list);
        
        if(a->label != label) {
            label = a->label;
            fprintf(f, "\nLABEL%02d ", label);
        } else if(label != 0) {
            fprintf(f, "\t\t");
        }
        
        switch(a->type) {
            case IT_NOP:
                fprintf(f, "\t\tNOP 0 0 0\n");
                break;
            case IT_A:
                fprintf(f, "\t\t%s%s%s%s R%d R%d", 
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
                fprintf(f, "\t\t%s R%d R%d %s\n", 
                    get_asm_mnemonic(a->op),
                    a->rd,
                    a->rs1,
                    immediate);
                free(immediate);
                break;
            }
            case IT_J:
                fprintf(f, "\t\tJ LABEL%02d\n", a->immediate->value.constant);
                break;
            case IT_B:
                fprintf(f, "\t\t%s%s%s", 
                    get_asm_mnemonic(a->op),
                    get_condition_str(a->cond),
                    a->c ? "C" : "");
                if(a->immediate) {
                    char* immediate = create_location_str(a->immediate);
                    fprintf(f, " %s", immediate);
                    free(immediate);
                }
                fprintf(f, "\n");
                break;
        }

        asm_list = ll_next(asm_list);
    }
    
    //Arguments:
    //R1 = A0 = 1st operand
    //R2 = A1 = 2nd operand
    
    //Other Registers Used:
    //T1, T3, T4, T5
    
    //Return:
    //T2  = result 

    if(USED_MULTIPLY) {
    
        fprintf(f, "\nmultiply");
        fprintf(f, "\t\tSUBS T2 R1 R0\n");
        fprintf(f, "\t\t\t\tBPZ checkPos2\n");
        fprintf(f, "\t\t\t\tADD R19 R1 R0\n");
        fprintf(f, "\t\t\t\tADD R1 R20 R0\n");
        fprintf(f, "\t\t\t\tADDI T3 R0 #1\n");
        fprintf(f, "checkPos2");
        fprintf(f, "\t\tSUBS T2 R2 R0\n");
        fprintf(f, "\t\t\t\tBPZ doMult\n");
        fprintf(f, "\t\t\t\tADD R19 R2 R0\n");
        fprintf(f, "\t\t\t\tADD T1 R20 R0\n");
        fprintf(f, "\t\t\t\tXOR T3 T3 T3\n");
        fprintf(f, "doMult");
        fprintf(f, "\t\t\tXOR T2 T2 T2\n");
        fprintf(f, "\t\t\t\tADDI T4 T4 #1\n");
        fprintf(f, "MultLoop");
        fprintf(f, "\t\tADD T2 T2 R1\n");
        fprintf(f, "\t\t\t\tSUBS R2 R2 T4\n");
        fprintf(f, "\t\t\t\tBNE MultLoop\n");
        fprintf(f, "\t\t\t\tSUBS T3 T3 R0\n");
        fprintf(f, "\t\t\t\tBEQ multDone\n");
        fprintf(f, "\t\t\t\tADD R19 T2 R0\n");
        fprintf(f, "\t\t\t\tADD T2 R20 R0\n");
        fprintf(f, "multDone");
        fprintf(f, "\t\tADD R0 R0 R0\n");
        fprintf(f, "\t\t\t\tBR");
    }
    
    
	//Arguments:
	//R1 = AO = dividend , R2 = A1 = divisor

	//Other Registers Used:
	//T0 =  = counter for 15 bit division, initially 15
	//T1 =  =quotient 
	//T2 =  =remainder 
	//T3 =  =dividend's negative flag
	//T4 =  =divisors neg flag 
	//T5 =  =logical AND and logical OR of T3 & T4
	//T6 = scratch

	//Return:
	//R13 = S0 = quotient
	//R14 = S1 = remainder
	
    if(USED_DIVIDE) {

        fprintf(f,"\ndiv_and_mod");
        fprintf(f,"\t\tADDI T3 R0 #0\n");
        fprintf(f,"\t\t\t\tADDI T4 R0 #0\n");
        fprintf(f,"\t\t\t\tSUBS T6 R1 R0\n");
        fprintf(f,"\t\t\t\tBPZ checkDivisorNeg\n");
        fprintf(f,"\t\t\t\tADDI T3 R0 #1\n");
        fprintf(f,"\t\t\t\tADDI R19 R1 #0\n");
        fprintf(f,"\t\t\t\tADDI R1 R20 #0\n");
        fprintf(f,"checkDivisorNeg");
        fprintf(f,"\tSUBS T6 R2 R0\n");
        fprintf(f,"\t\t\t\tBPZ doDivision\n");
        fprintf(f,"\t\t\t\tADDI T4 R0 #1\n");
        fprintf(f,"\t\t\t\tADDI R19 R2 #0\n");
        fprintf(f,"\t\t\t\tADDI T2 R20 #0\n");
        fprintf(f,"doDivision");
        fprintf(f,"\t\tADDI T0 R0 #15\n");
        fprintf(f,"\t\t\t\tADDI T1 R0 #0\n");
        fprintf(f,"\t\t\t\tLSL R2 R2 T0\n");
        fprintf(f,"\t\t\t\tADDI T2 R1 #0\n");
        fprintf(f,"divisionLoop");
        fprintf(f,"\tSUBS T2 T2 R2\n");
        fprintf(f,"\t\t\t\tBLT remainderLT0\n");
        fprintf(f,"\t\t\t\tADDI T6 R0 #1\n");
        fprintf(f,"\t\t\t\tLSL T1 T1 T6\n");
        fprintf(f,"\t\t\t\tADDI T1 T1 #1\n");
        fprintf(f,"divisorRS");
        fprintf(f,"\t\tLSR R2 R2 T6\n");
        fprintf(f,"\t\t\t\tSUBS T6 T0 R0\n");
        fprintf(f,"\t\t\t\tBLE exitDivision\n");
        fprintf(f,"\t\t\t\tSUBI T0 T0 #1\n");
        fprintf(f,"\t\t\t\tB divisionLoop\n");
        fprintf(f,"remainderLT0");
        fprintf(f,"\tADD T2 T2 R2\n");
        fprintf(f,"\t\t\t\tADDI T6 R0 #1\n");
        fprintf(f,"\t\t\t\tLSL T1 T1 T6\n");
        fprintf(f,"\t\t\t\tB divisorRS\n");
        fprintf(f,"exitDivision");
        fprintf(f,"\tOR T5 T3 T4\n");
        fprintf(f,"\t\t\t\tSUBS T6 T5 R0\n");
        fprintf(f,"\t\t\t\tBEQ divisionDone\n");
        fprintf(f,"\t\t\t\tAND T5 T3 T4\n");
        fprintf(f,"\t\t\t\tSUBS T6 T5 R0\n");
        fprintf(f,"\t\t\t\tBEQ divisionDone\n");
        fprintf(f,"\t\t\t\tADD R19 T1 R0\n");
        fprintf(f,"\t\t\t\tADD T1 R20 R0\n");
        fprintf(f,"divisionDone");
        fprintf(f,"\tADD R13 T1 R0\n");
        fprintf(f,"\t\t\t\tADD R14 T2 R0\n");
        fprintf(f,"\t\t\t\tBR\n");
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

	if (immediate->type == W_CONSTANT && immediate->value.constant > 0xFFFF){
		add_itype(label, ASM_ADDI, rd, ZERO, const_location(immediate->value.constant & 0xFFFF));
    	int im = immediate->value.constant >> 16;
    	add_itype(label, ASM_LUI, rd, ZERO, const_location(im));
    	add_atype(label, op, rd, rs1, rd, false, false, false);
    	return;
    	 
    }
    
    ASSEMBLY* a = new_asm(label);
    a->type = IT_I;
    a->op = op;
    a->rd = rd;
    a->rs1 = rs1;
    a->immediate = immediate ;
    
    
    
    assembly_code = ll_insertfront(assembly_code, a);
}

void add_jtype(int label, LOCATION* w) {
    ASSEMBLY* a = new_asm(label);
    a->type = IT_J;
    a->op = ASM_JMP;
    a->immediate = w;
    assembly_code = ll_insertfront(assembly_code, a);
}

void add_btype(int label, ASM_OP op, LOCATION* immediate, bool c, CONDITION cond) {
    ASSEMBLY* a = new_asm(label);
    a->type = IT_B;
    a->op = op;
    a->immediate = immediate;
    a->c = c;
    a->cond = cond;
    assembly_code = ll_insertfront(assembly_code, a);
}