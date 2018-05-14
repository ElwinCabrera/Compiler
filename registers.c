#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "registers.h"
#include "intermediate_code.h"
#include "symbol_table.h"
#include "address.h"
#include "location.h"
#include "assembly.h"

static REGISTER_DESCRIPTORS* _reg;

LINKED_LIST** get_register_descriptor(REG r) {
    if(!_reg) {
        _reg = malloc(sizeof(REGISTER_DESCRIPTORS));
        memset(_reg, 0, sizeof(REGISTER_DESCRIPTORS));
        for(int i = 0; i <= 31; i++) {
            _reg->registers[i] = NULL;
        }
    }

    if(r <= 31) {
        return &_reg->registers[r];
    } else {
        printf("Tried to access a register that doesn't exist.\n");
    }

    return NULL;
}

bool match_first_register(LINKED_LIST* l, void* _) {
    LOCATION* where = ll_value(l);
    return where && where->type == W_REGISTER;
}

bool match_not_this_register(LINKED_LIST* l, void* reg) {
    LOCATION* where = ll_value(l);
    return where->type != W_REGISTER || where->value.reg != (long) reg;
}

bool match_same_register(LINKED_LIST* l, void* reg) {
    LOCATION* where = ll_value(l);
    return where->type == W_REGISTER && where->value.reg == (long) reg;
}

void clear_temporary_registers() {

    for(int i = T0; i <= T7; i++) {
        remove_register_from_addresses(i);
    }

}

void remove_register_from_addresses(REG r) {
   if(r > 31 || r < 0) {
        printf("Tried to access a register that doesn't exist.\n");
        return;
    }
    LINKED_LIST** descriptor_ptr = get_register_descriptor(r);
    LINKED_LIST* dsc = *descriptor_ptr;

    while(dsc) {
        ADDRESS* adr = ll_value(dsc);
        adr->value.symbol->registers = adr->value.symbol->registers & ~(1 << r);
        dsc = ll_next(dsc);
    }
	
    ll_free(descriptor_ptr, false);
}

void add_address_to_descriptor(REG r, ADDRESS* a) {
    if(r > 31 || r < 0) {
        printf("Tried to access a register that doesn't exist.\n");
        return;
    }
    
    a->value.symbol->registers |= (1 << r);
    _reg->registers[r] = ll_insertfront(_reg->registers[r], a);
}

REG get_dest_register(int block, ADDRESS* a) {
    if(!a) {
        printf("Tried to create a destination register for a null address.\n");
        return NO_REGISTER;
    }

    if(a->meta == AT_LABEL) {
        return NO_REGISTER;
    }

    if(a->meta != AT_SYMBOL && a->meta != AT_TEMPORARY) {
        printf("Tried to create a destination register for a non-symbol.\n");
        return NO_REGISTER;
    }

    int where = a->value.symbol->registers;

    if(where) {
        for(int i = 0; i <= 31; i++) {
            if(where & 1) {
                return i;
            }
            where = where >> 1;
        }
    }

    REG choice = NO_REGISTER;
    int min_spills = INT_MAX;

    for(int i = T0; i <= S5; i++) {
        LINKED_LIST* descriptor = *get_register_descriptor(i);
        if(!descriptor) {
            add_address_to_descriptor(i, a);
            return i;
        }
        
        int length = ll_length(descriptor);
        if(length < min_spills) {
            min_spills = length;
            choice = i;
        }
    }

    remove_register_from_addresses(choice);
    add_address_to_descriptor(choice, a);

    return choice;
}

REG get_source_register(int block, ADDRESS* a) {
    
    if(!a) {
        return NULL_ADDRESS;
    }

    if(a->meta != AT_SYMBOL && a->meta != AT_TEMPORARY) {
        return CONST_VALUE;
    }

    int where = a->value.symbol->registers;

    if(where) {
        for(int i = 0; i <= 31; i++) {
            if(where & 1) {
                return i;
            }
            where = where >> 1;
        }
    }

    REG choice = NO_REGISTER;
    int min_spills = INT_MAX;

    for(int i = T0; i <= S5; i++) {
        LINKED_LIST* descriptor = *get_register_descriptor(i);
        
        if(!descriptor) {
            add_address_to_descriptor(i, a);
            load_into_register(block, i, a);
            return i;
        }

        int length = ll_length(descriptor);
        if(length < min_spills) {
            min_spills = length;
            choice = i;
        }
    }

    remove_register_from_addresses(choice);
    add_address_to_descriptor(choice, a);
    LOCATION* l = register_location(choice);
    a->value.symbol->address_descriptor = ll_insertfront(a->value.symbol->address_descriptor, l);

    return choice;
}
