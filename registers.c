#include <stdlib.h>
#include <string.h>
#include "registers.h"
#include "intermediate_code.h"
#include "symbol_table.h"

static REGISTER_DESCRIPTORS* _reg;

LINKED_LIST* get_register_descriptor(REG r) {
    if(!_reg) {
        _reg = malloc(sizeof(REGISTER_DESCRIPTORS));
        memset(_reg, 0, sizeof(REGISTER_DESCRIPTORS));
    }

    if(r <= 31) {
        return _reg->registers[r];
    } else {
        printf("Tried to access a register that doesn't exist.\n");
    }

    return NULL;
}

void get_reg(TAC* code, int* rd, int* rs1, int* rs2) {

}