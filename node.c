#include <string.h>
#include "node.h"
#include "types.h"
#include "symbolTable.h"

static int node_count = 0;

NODE* new_node() {
    NODE* n = malloc(sizeof(NODE));
    n->id = node_count++;
    return n;
}

void free_node(NODE* n) {
    free(n->type_name);
    free(n);
}

NODE* int_node(int value) {
    NODE* n = new_node();
    n->meta = INT_CONSTANT;
    n->type_name = strdup("integer");
    n->value.integer = value;
    return n;
}

NODE* boolean_node(int value) {
    NODE* n = new_node();
    n->meta = BOOLEAN_CONSTANT;
    n->type_name = strdup("Boolean");
    n->value.boolean = value;
    return n;
}

NODE* real_node(double value) {
    NODE* n = new_node();
    n->meta = REAL_CONSTANT;
    n->type_name = strdup("real");
    n->value.real = value;
    return n;
}

NODE* char_node(char value) {
    NODE* n = new_node();
    n->meta = CHAR_CONSTANT;
    n->type_name = strdup("character");
    n->value.character = value;
    return n;
}

NODE* string_node(char* value) {
    NODE* n = new_node();
    n->meta = STRING_CONSTANT;
    n->type_name = strdup("string");
    n->value.string = value;
    return n;
}

NODE* null_node() {
    NODE* n = new_node();
    n->meta = NULL_CONSTANT;
    return n;
}

NODE* symbol_node(struct symtab* value) {
    NODE* n = new_node();
    n->meta = SYMBOL;
    if(value && value->type) { 
        n->type_name = strdup(value->type->name);
    } else {
        n->type_name = NULL;
    }
    n->value.symbol = value;
    return n;
}

NODE* ir_node(int i, char* type_name) {
    NODE* n = new_node();
    n->meta = CODE;
    if(type_name) {
        n->type_name = strdup(type_name);
    } else {
        n->type_name = NULL;
    }
    n->value.instruction = i;
    return n;
}

char* node_to_string(NODE* n) {
    if(!n) {
        return NULL;
    }

    switch(n->meta) {
        case STRING_CONSTANT:
            return n->value.string;
        case CHAR_CONSTANT: {
            // Mem leak
            char * tmp = malloc(2 * sizeof(char));
            sprintf(tmp, "%c", n->value.character);
            return tmp;
        }
        case INT_CONSTANT: {
            // Mem leak
            char * tmp = malloc(20 * sizeof(char));
            sprintf(tmp, "%d", n->value.integer);
            return tmp;
        }
        case REAL_CONSTANT: {
            // Mem leak
            char * tmp = malloc(20 * sizeof(char));
            sprintf(tmp, "%f", n->value.real);
            return tmp;
        }
        case NULL_CONSTANT:
            return "null";
        case BOOLEAN_CONSTANT:
            return n->value.boolean ? "true" : "false";
        case SYMBOL:
            return n ? (n->value.symbol ? n->value.symbol->name : NULL) : NULL;
        case CODE: {
            // Mem leak
            char * tmp = malloc(10 * sizeof(char));
            sprintf(tmp, "$%d", n->value.instruction);
            return tmp;
        }
        default:
            return NULL;
    }
}