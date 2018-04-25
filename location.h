#ifndef LOCATION_H
#define LOCATION_H

#include "registers.h"
#include "data_block.h"

typedef enum where {
    W_REGISTER,
    W_BLOCK,
    W_MEMORY,
    W_CONSTANT,
} WHERE;

typedef struct location {
    WHERE type;
    union {
        REG reg;
        DATA* data;
        int pointer;
        int constant;
    } value;
} LOCATION;

LOCATION* register_location(REG);
LOCATION* data_location(DATA*);
LOCATION* memory_location(int);
LOCATION* const_location(int);
char* create_location_str(LOCATION*);

#endif