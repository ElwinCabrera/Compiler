#ifndef REGISTERS_H
#define REGISTERS_H

#include "linked_list.h"
#include "symbol_table.h"
#include "intermediate_code.h"

typedef struct register_destricptors {
    struct linked_list* registers[31];
} REGISTER_DESCRIPTORS;

typedef enum reg {
    ZERO,
    ARG0,
    ARG1,
    ARG2,
    ARG3,
    T0,
    T1,
    T2,
    T3,
    T4,
    T5,
    T6,
    T7,
    S0,
    S1,
    S2,
    S3,
    S4,
    S5,
    POSNEG,
    NEGPOS,
    COUNTER_P,
    COUNTER_M,
    HEAP,
    FRAME,
    STACK,
    LINK0,
    LINK1,
    LINK2,
    LINK3,
    PC,
    CPSR,
} REG;

LINKED_LIST* get_register_descriptor(REG);
void add_variable_to_register(REG, struct symbol*);
void remove_variable_from_register(REG, struct symbol*);
void get_reg(struct tac*, int*, int*, int*);

#endif
