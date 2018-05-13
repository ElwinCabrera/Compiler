#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include "registers.h"
#include "code_blocks.h"
#include "intermediate_code.h"

typedef enum asm_op {

    // A Type
    ASM_NOP,    // Nothing
    ASM_ADD,    // RD = RS1 + RS2
    ASM_AND,    // RD = RS1 & RS2
    ASM_ASR,    // RD = RS1 << RS2
    ASM_LSL,    // RD = RS1 >> RS2
    ASM_LSR,    // RD = RS1 << RS2
    ASM_NAND,   // RD = ~(RS1 & RS2)
    ASM_NOR,    // RD = ~(RS1 | RS2)
    ASM_NOT,    // RD = ~(RS1)
    ASM_OR,     // RD = RS1 | RS2
    ASM_SUB,    // RD = RS1 - RS2
    ASM_XNOR,   // RD = RS1 ⊙ RS2
    ASM_XOR,    // RD = RS1 ⊕ RS2

    // I TYPE

    ASM_ADDI,   // RD + RS + #Imm
    ASM_ANDI,   // RD + RS & #Imm
    ASM_ASRI,   // RD = RS1 >> #Imm
    ASM_LDR,    // RD = [RS + #Imm]
    ASM_LDRB,   // RD[7:0] = [RS + #Imm]
    ASM_LDRBU,  // RD[7:0] = [RS + #Imm]
    ASM_LDRH,   // RD[15:0] = [RS + #Imm] 
    ASM_LDRHU,  // RD[15:0] = [RS + #Imm]
    ASM_LSLI,   // RD = RS1 << #Imm
    ASM_LSRI,   // RD = RS1 >> #Imm
    ASM_LUI,    // RD[31:16] = RS1 >> #Imm
    ASM_NANDI,  // RD = ~(RS1 & )
    ASM_NORI,   // RD = ~(RS1 | #Imm)
    ASM_ORI,    // RD = RS1 | #Imm
    ASM_STR,    // [RD] = RS + #Imm
    ASM_STRB,   // [RD] = (RS + #Imm)[7:0]
    ASM_STRH,   // [RD] = (RS + #Imm)[15:0]
    ASM_SUBI,   // RD = RS1 - #Imm
    ASM_XNORI,  // RD = RS1 ⊙ #Imm
    ASM_XORI,   // RD = RS1 ⊕ #Imm

    // J Type

    ASM_JMP,

    // B Type

    ASM_BRANCH,     
    ASM_BRANCHL,
    ASM_BRANCHR,

} ASM_OP;

typedef enum condition_code {
    AL, //Always
    EQ, //Z-Set
    NE, //Z-Clear
    CA, //C-Set
    CC, //C-Clear
    NG, //N-Set
    PZ, //N-Clear
    VS, //V-Set
    VC, //V-Clear
    HI, //C-Set Z-Clear
    LS, //C-Clear || Z-Set
    GE, //N == V
    LT, //N != V
    GT, //Z-Clear, N=V
    LE, //Z-Set || N != V
} CONDITION;

typedef enum i_type {
    IT_A = 1,
    IT_I,
    IT_J,
    IT_B,
    IT_NOP,
} I_TYPE;

typedef struct assembly {
    int label;
    I_TYPE type; 
    ASM_OP op;
    REG rd;
    REG rs1;
    REG rs2;
    bool s;
    bool c;
    CONDITION cond;
    struct location* immediate;
} ASSEMBLY;

const char* get_asm_mnemonic(ASM_OP);
const char* get_condition_str(CONDITION);
void create_assembly_block(struct block*);
void create_assembly(int, struct tac*);
void add_nop(int);
void add_atype(int, ASM_OP, REG, REG, REG, bool, bool, CONDITION);
void add_itype(int, ASM_OP, REG, REG, struct location*);
void add_jtype(int, struct location*);
void add_btype(int, ASM_OP, struct location*, bool, CONDITION);
void process_code_blocks(LINKED_LIST*);
void print_asm_code(FILE*);


#endif
