#include <stdlib.h>
#include <string.h>
#include "registers.h"
#include "intermediate_code.h"
#include "symbol_table.h"
#include "address.h"
#include "location.h"

static REGISTER_DESCRIPTORS* _reg;

LINKED_LIST** get_register_descriptor(REG r) {
    if(!_reg) {
        _reg = malloc(sizeof(REGISTER_DESCRIPTORS));
        memset(_reg, 0, sizeof(REGISTER_DESCRIPTORS));
    }

    if(r <= 31) {
        return &_reg->registers[r];
    } else {
        printf("Tried to access a register that doesn't exist.\n");
    }

    return NULL;
}

void clear_temporary_registers() {

    for(int i = T0; i <= T7; i++) {
        ll_free(get_register_descriptor(i));
    }

}

void add_address_to_descriptor(REG r, ADDRESS* a) {
    if(r <= 31) {
        _reg->registers[r] = ll_insertfront(_reg->registers[r], a);
    } else {
        printf("Tried to access a register that doesn't exist.\n");
    }
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

REG get_parameter_register(ADDRESS* a) {
    if(!a) {
        return NULL_ADDRESS;
    }

    for(int i = ARG0; i <= ARG3; i++) {
        LINKED_LIST* descriptor = *get_register_descriptor(i);
        if(!descriptor) {
            add_address_to_descriptor(i, a);
            LOCATION* l = register_location(i);
            a->value.symbol->address_descriptor = ll_insertfront(a->value.symbol->address_descriptor, l);
            return i;
        } else if(ll_find(descriptor, a, pointer_match)) {
            return i;
        }
    }

    return NO_REGISTER;
}

REG get_dest_register(ADDRESS* a) {
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

    LINKED_LIST* locations = a->value.symbol->address_descriptor;

    while(locations) {
        LOCATION* where = ll_value(locations);
        if(where->type == W_REGISTER) {
            LINKED_LIST* reg_desc = *get_register_descriptor(where->value.reg);
            if(ll_length(reg_desc) == 1) {
                return where->value.reg;
            }
        }
        locations = ll_next(locations);
    }

    for(int i = T0; i <= S5; i++) {
        LINKED_LIST* descriptor = *get_register_descriptor(i);
        if(!descriptor) {
            add_address_to_descriptor(i, a);
            LOCATION* l = register_location(i);
            a->value.symbol->address_descriptor = ll_insertfront(a->value.symbol->address_descriptor, l);
            return i;
        }
    }

    return ZERO;
}

REG get_source_register(ADDRESS* a) {
    
    if(!a) {
        return NULL_ADDRESS;
    }

    if(a->meta != AT_SYMBOL && a->meta != AT_TEMPORARY) {
        return CONST_ADDRESS;
    }

    LOCATION* where = ll_find(a->value.symbol->address_descriptor, NULL, match_first_register);

    if(where) {
        return where->value.reg;
    }

    for(int i = T0; i <= S5; i++) {
        LINKED_LIST* descriptor = *get_register_descriptor(i);
        if(!descriptor) {
            add_address_to_descriptor(i, a);
            LOCATION* l = register_location(i);
            a->value.symbol->address_descriptor = ll_insertfront(a->value.symbol->address_descriptor, l);
            return i;
        } else if(ll_find(descriptor, a, pointer_match)) {
            return i;
        }
    }

    // No registers available
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

    for(int i = T0; i <= S5; i++) {
        LINKED_LIST** reg_desc_ptr = get_register_descriptor(i);
        ADDRESS* addr = ll_value(*reg_desc_ptr);
        LINKED_LIST** addr_desc_ptr = &addr->value.symbol->address_descriptor;
        if(ll_find(*addr_desc_ptr, (void*) i, match_not_this_register)) {
            ll_remove(addr_desc_ptr,  (void*) i, match_same_register);
            
            ll_free(reg_desc_ptr);
            *reg_desc_ptr = ll_insertfront(*reg_desc_ptr, a);
            LOCATION* l = register_location(i);
            a->value.symbol->address_descriptor = ll_insertfront(a->value.symbol->address_descriptor, l);
            return i;
        }
    }

#pragma GCC diagnostic pop


    return ZERO;
}